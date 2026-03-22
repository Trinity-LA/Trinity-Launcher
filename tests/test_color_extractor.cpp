#include "TrinityLib/core/color_extractor.hpp"

#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        qCritical() << "Usage: test_color_extractor <image_path> [num_colors]";
        qCritical() << "  Example: test_color_extractor /path/to/wallpaper.jpg 6";
        return 1;
    }

    QString imagePath = QString::fromLocal8Bit(argv[1]);
    int numColors = (argc >= 3) ? QString(argv[2]).toInt() : 6;

    if (numColors < 1) numColors = 6;

    qDebug() << "========================================";
    qDebug() << "  Mini-Matugen — Color Extractor Test";
    qDebug() << "========================================";
    qDebug() << "Image:" << imagePath;
    qDebug() << "Requesting" << numColors << "dominant colors...";
    qDebug() << "";

    auto colors = ColorExtractor::extractColors(imagePath, numColors);

    if (colors.isEmpty()) {
        qCritical() << "ERROR: Could not extract any colors from the image.";
        return 1;
    }

    qDebug() << "Extracted" << colors.size() << "dominant colors:";
    qDebug() << "----------------------------------------";

    for (int i = 0; i < colors.size(); ++i) {
        const auto &ci = colors[i];
        qDebug().noquote()
            << QString("  Color #%1: %2  (RGB: %3, %4, %5)  — %6%")
                   .arg(i + 1, 2)
                   .arg(ci.color.name().toUpper())
                   .arg(ci.color.red(),   3)
                   .arg(ci.color.green(), 3)
                   .arg(ci.color.blue(),  3)
                   .arg(ci.percentage, 0, 'f', 1);
    }

    qDebug() << "----------------------------------------";
    qDebug() << "Done!";

    return 0;
}
