#include "CompactMenuBarStyle.h"

#include <QSize>
#include <QStyleOption>

namespace ui::common {

CompactMenuBarStyle::CompactMenuBarStyle(const QString& baseStyleName)
    : QProxyStyle(baseStyleName) {
}

int CompactMenuBarStyle::pixelMetric(PixelMetric metric,
                                     const QStyleOption* option,
                                     const QWidget* widget) const {
    const int base = QProxyStyle::pixelMetric(metric, option, widget);

    switch (metric) {
    case PM_MenuBarHMargin:
    case PM_MenuBarVMargin:
    case PM_MenuBarItemSpacing:
        return qMax(0, base / 2);
    default:
        return base;
    }
}

QSize CompactMenuBarStyle::sizeFromContents(ContentsType type,
                                            const QStyleOption* option,
                                            const QSize& contentsSize,
                                            const QWidget* widget) const {
    QSize size = QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
    if (type == CT_MenuBarItem) {
        const int horizontalShrink = qMax(4, size.height() / 5);
        size.setWidth(qMax(contentsSize.width(), size.width() - horizontalShrink));
    }
    return size;
}

} // namespace ui::common
