#include "ThemeVisuals.h"
#include <QApplication>
#include <QColor>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

namespace {

QString toCssColor(const QColor& color) {
    return color.name(color.alpha() < 255 ? QColor::HexArgb : QColor::HexRgb);
}

void drawSun(QPainter& painter, const QRectF& rect, const QColor& color, qreal strokeWidth) {
    const QPointF center = rect.center();
    const qreal radius = rect.width() * 0.20;
    const qreal rayInner = rect.width() * 0.31;
    const qreal rayOuter = rect.width() * 0.45;
    const QPen pen(color, strokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    painter.save();
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(center, radius, radius);

    const std::array<QPointF, 8> directions = {
        QPointF(0.0, -1.0), QPointF(0.707, -0.707), QPointF(1.0, 0.0), QPointF(0.707, 0.707),
        QPointF(0.0, 1.0), QPointF(-0.707, 0.707), QPointF(-1.0, 0.0), QPointF(-0.707, -0.707)
    };

    for (const QPointF& direction : directions) {
        painter.drawLine(center + direction * rayInner, center + direction * rayOuter);
    }
    painter.restore();
}

void drawMoon(QPainter& painter, const QRectF& rect, const QColor& color) {
    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);

    QPainterPath moonPath;
    moonPath.addEllipse(rect);

    QPainterPath cutoutPath;
    const QRectF cutout(rect.left() + rect.width() * 0.30,
                        rect.top() + rect.height() * 0.08,
                        rect.width() * 0.78,
                        rect.height() * 0.78);
    cutoutPath.addEllipse(cutout);
    painter.drawPath(moonPath.subtracted(cutoutPath));
    painter.restore();
}

} // namespace

namespace ui::common {

QIcon ThemeVisuals::buildModeIcon(const QPalette& palette, Theme::Mode mode, int baseSize) {
    const QColor primaryColor = palette.color(QPalette::ButtonText);
    const QColor accentColor = palette.color(QPalette::Highlight);
    QIcon icon;

    const QList<int> sizes = {
        qMax(20, baseSize),
        qMax(24, baseSize + 2),
        qMax(28, baseSize + 4),
        qMax(32, baseSize + 8)
    };

    for (const int size : sizes) {
        QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        const qreal strokeWidth = qMax(1.8, static_cast<double>(size) / 14.0);
        const QRectF bounds(strokeWidth, strokeWidth, size - (strokeWidth * 2.0), size - (strokeWidth * 2.0));

        switch (mode) {
        case Theme::Mode::Auto:
            drawSun(painter,
                    QRectF(size * 0.01, size * 0.01, size * 0.46, size * 0.46),
                    primaryColor,
                    qMax(1.4, strokeWidth * 0.8));
            drawMoon(painter,
                     QRectF(size * 0.53, size * 0.53, size * 0.46, size * 0.46),
                     primaryColor);
            painter.setPen(QPen(accentColor, strokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.drawLine(QPointF(size * 0.20, size * 0.82), QPointF(size * 0.82, size * 0.20));
            break;
        case Theme::Mode::Light:
            drawSun(painter, bounds, primaryColor, strokeWidth);
            break;
        case Theme::Mode::Dark:
            drawMoon(painter, bounds.adjusted(size * 0.06, size * 0.02, -size * 0.06, -size * 0.02), primaryColor);
            break;
        }

        painter.end();
        icon.addPixmap(QPixmap::fromImage(image));
    }

    return icon;
}

QString ThemeVisuals::navigationListStyle(const QPalette& palette) {
    QColor itemHover = palette.color(QPalette::Highlight);
    itemHover.setAlpha(24);

    QColor itemSelected = palette.color(QPalette::Highlight);
    itemSelected.setAlpha(40);

    const int fontHeight = qApp ? qApp->fontMetrics().height() : 14;
    const int itemPadding = qMax(8, fontHeight / 2);
    const int selectionBorderWidth = qMax(2, fontHeight / 5);

    return QString::fromLatin1(R"(
        QListWidget {
            background-color: %1;
            color: %2;
            border: none;
            border-right: 1px solid %3;
        }
        QListWidget::item {
            padding: %4px;
            border-left: %5px solid transparent;
        }
        QListWidget::item:hover {
            background-color: %6;
        }
        QListWidget::item:selected {
            background-color: %7;
            color: %8;
            border-left: %5px solid %9;
        }
    )")
        .arg(toCssColor(palette.color(QPalette::Window)))
        .arg(toCssColor(palette.color(QPalette::WindowText)))
        .arg(toCssColor(palette.color(QPalette::Mid)))
        .arg(itemPadding)
        .arg(selectionBorderWidth)
        .arg(toCssColor(itemHover))
        .arg(toCssColor(itemSelected))
        .arg(toCssColor(palette.color(QPalette::HighlightedText)))
        .arg(toCssColor(palette.color(QPalette::Highlight)));
}

} // namespace ui::common
