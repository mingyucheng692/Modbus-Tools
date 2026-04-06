#pragma once

#include <QProxyStyle>

namespace ui::common {

class CompactMenuBarStyle final : public QProxyStyle {
public:
    explicit CompactMenuBarStyle(const QString& baseStyleName = QString());

    int pixelMetric(PixelMetric metric,
                    const QStyleOption* option = nullptr,
                    const QWidget* widget = nullptr) const override;

    QSize sizeFromContents(ContentsType type,
                           const QStyleOption* option,
                           const QSize& contentsSize,
                           const QWidget* widget = nullptr) const override;
};

} // namespace ui::common
