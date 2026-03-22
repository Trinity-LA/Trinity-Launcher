#pragma once

#include <QColor>
#include <QString>
#include <QVector>

/**
 * @brief Extracts dominant colors from an image using k-means clustering.
 *
 * Mini-matugen implementation: analyzes a wallpaper image and returns
 * the most prominent colors with their relative percentages.
 */
class ColorExtractor {
public:
    /**
     * @brief Holds a dominant color and its relative weight.
     */
    struct ColorInfo {
        QColor color;
        double percentage; ///< Percentage of pixels belonging to this cluster
    };

    /**
     * @brief Extract the N most dominant colors from an image file.
     *
     * The image is downscaled internally for performance. Transparent
     * pixels are ignored.
     *
     * @param imagePath  Absolute path to the image (PNG, JPG, WEBP, etc.)
     * @param numColors  Number of dominant colors to extract (default 6)
     * @return Sorted vector of ColorInfo (highest percentage first),
     *         or empty vector on error.
     */
    static QVector<ColorInfo> extractColors(const QString &imagePath,
                                            int numColors = 6);

private:
    /**
     * @brief Run k-means clustering on a set of pixel colors.
     */
    static QVector<ColorInfo> kMeansClustering(const QVector<QColor> &pixels,
                                               int k,
                                               int maxIterations = 20);
};
