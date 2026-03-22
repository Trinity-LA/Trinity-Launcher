#include "TrinityLib/core/color_extractor.hpp"

#include <QDebug>
#include <QImage>
#include <QRandomGenerator>

#include <algorithm>
#include <cmath>
#include <limits>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Euclidean distance² in RGB space (no sqrt needed for comparisons)
static inline double colorDistanceSq(const QColor &a, const QColor &b) {
    double dr = a.redF()   - b.redF();
    double dg = a.greenF() - b.greenF();
    double db = a.blueF()  - b.blueF();
    return dr * dr + dg * dg + db * db;
}

// ---------------------------------------------------------------------------
// extractColors
// ---------------------------------------------------------------------------

QVector<ColorExtractor::ColorInfo>
ColorExtractor::extractColors(const QString &imagePath, int numColors) {
    QImage img(imagePath);
    if (img.isNull()) {
        qWarning() << "[ColorExtractor] Could not load image:" << imagePath;
        return {};
    }

    // Down-scale to ~100x100 for speed
    constexpr int TARGET_SIZE = 100;
    if (img.width() > TARGET_SIZE || img.height() > TARGET_SIZE) {
        img = img.scaled(TARGET_SIZE, TARGET_SIZE, Qt::KeepAspectRatio,
                         Qt::FastTransformation);
    }

    // Convert to ARGB32 for uniform pixel access
    img = img.convertToFormat(QImage::Format_ARGB32);

    // Sample all non-transparent pixels
    QVector<QColor> pixels;
    pixels.reserve(img.width() * img.height());

    for (int y = 0; y < img.height(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(img.constScanLine(y));
        for (int x = 0; x < img.width(); ++x) {
            QColor c(line[x]);
            if (c.alpha() < 128)
                continue; // skip mostly-transparent pixels
            pixels.append(c);
        }
    }

    if (pixels.isEmpty()) {
        qWarning() << "[ColorExtractor] No opaque pixels found in image.";
        return {};
    }

    qDebug() << "[ColorExtractor] Sampled" << pixels.size()
             << "pixels from" << img.width() << "x" << img.height()
             << "image";

    return kMeansClustering(pixels, numColors);
}

// ---------------------------------------------------------------------------
// kMeansClustering
// ---------------------------------------------------------------------------

QVector<ColorExtractor::ColorInfo>
ColorExtractor::kMeansClustering(const QVector<QColor> &pixels, int k,
                                 int maxIterations) {
    const int n = pixels.size();
    if (n == 0 || k <= 0)
        return {};

    // Clamp k to the number of unique-ish pixels
    k = std::min(k, n);

    // --- Initialize centroids with random distinct pixels (k-means++) lite ---
    QVector<QColor> centroids;
    centroids.reserve(k);

    auto *rng = QRandomGenerator::global();

    // First centroid: random pixel
    centroids.append(pixels[rng->bounded(n)]);

    // Subsequent centroids: pick the pixel farthest from existing centroids
    for (int c = 1; c < k; ++c) {
        double bestDist = -1.0;
        int bestIdx = 0;
        for (int i = 0; i < n; ++i) {
            double minDist = std::numeric_limits<double>::max();
            for (const QColor &cent : centroids) {
                double d = colorDistanceSq(pixels[i], cent);
                if (d < minDist) minDist = d;
            }
            if (minDist > bestDist) {
                bestDist = minDist;
                bestIdx = i;
            }
        }
        centroids.append(pixels[bestIdx]);
    }

    // --- Iterative assignment & update ---
    QVector<int> assignments(n, 0);

    for (int iter = 0; iter < maxIterations; ++iter) {
        bool changed = false;

        // Assign each pixel to nearest centroid
        for (int i = 0; i < n; ++i) {
            double bestDist = std::numeric_limits<double>::max();
            int bestK = 0;
            for (int c = 0; c < k; ++c) {
                double d = colorDistanceSq(pixels[i], centroids[c]);
                if (d < bestDist) {
                    bestDist = d;
                    bestK = c;
                }
            }
            if (assignments[i] != bestK) {
                assignments[i] = bestK;
                changed = true;
            }
        }

        if (!changed)
            break; // converged

        // Recompute centroids
        QVector<double> sumR(k, 0), sumG(k, 0), sumB(k, 0);
        QVector<int> count(k, 0);

        for (int i = 0; i < n; ++i) {
            int c = assignments[i];
            sumR[c] += pixels[i].redF();
            sumG[c] += pixels[i].greenF();
            sumB[c] += pixels[i].blueF();
            count[c]++;
        }

        for (int c = 0; c < k; ++c) {
            if (count[c] > 0) {
                centroids[c] = QColor::fromRgbF(sumR[c] / count[c],
                                                sumG[c] / count[c],
                                                sumB[c] / count[c]);
            }
        }
    }

    // --- Build result sorted by percentage ---
    QVector<int> clusterCount(k, 0);
    for (int i = 0; i < n; ++i)
        clusterCount[assignments[i]]++;

    QVector<ColorInfo> result;
    result.reserve(k);
    for (int c = 0; c < k; ++c) {
        if (clusterCount[c] == 0)
            continue;
        result.append({centroids[c],
                       100.0 * clusterCount[c] / static_cast<double>(n)});
    }

    std::sort(result.begin(), result.end(),
              [](const ColorInfo &a, const ColorInfo &b) {
                  return a.percentage > b.percentage;
              });

    return result;
}
