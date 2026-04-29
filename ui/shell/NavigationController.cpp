#include "shell/NavigationController.h"

#include "AppConstants.h"

#include <QColor>
#include <QFontMetrics>
#include <QIcon>
#include <QImage>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QStackedWidget>
#include <QStyle>
#include <QToolButton>
#include <QWidget>
#include <iterator>

namespace {

struct NavigationItemMetadata {
    const char* iconPath;
    const char* accentColor;
};

constexpr NavigationItemMetadata kNavigationItems[] = {
    {":/assets/Modbus-TCP.ico", "#3B82F6"},
    {":/assets/Modbus-RTU.ico", "#8B5CF6"},
    {":/assets/TCP-Client.ico", "#06B6D4"},
    {":/assets/Serial-Port.ico", "#F59E0B"},
    {":/assets/Frame-Analyzer.ico", "#10B981"},
};

QIcon buildNavigationIcon(const QString& resourcePath, const QColor& accentColor) {
    QPixmap sourcePixmap;
    const QIcon sourceIcon(resourcePath);
    const QList<QSize> candidateSizes = sourceIcon.availableSizes();
    if (!candidateSizes.isEmpty()) {
        QSize best = candidateSizes.first();
        for (const QSize& size : candidateSizes) {
            if (size.width() * size.height() > best.width() * best.height()) {
                best = size;
            }
        }
        sourcePixmap = sourceIcon.pixmap(best);
    } else {
        sourcePixmap.load(resourcePath);
    }
    if (sourcePixmap.isNull()) {
        sourcePixmap = sourceIcon.pixmap(256, 256);
    }
    if (sourcePixmap.isNull()) {
        return QIcon();
    }

    QImage image = sourcePixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QColor pixel = QColor::fromRgba(image.pixel(x, y));
            if (pixel.alpha() == 0) {
                continue;
            }
            if (pixel.red() > 245 && pixel.green() > 245 && pixel.blue() > 245) {
                pixel.setAlpha(0);
                image.setPixelColor(x, y, pixel);
                continue;
            }
            const int lightness = qBound(35, qGray(pixel.red(), pixel.green(), pixel.blue()), 215);
            QColor recolored = accentColor;
            recolored.setHsl(accentColor.hslHue(), qMax(accentColor.hslSaturation(), 120), lightness, pixel.alpha());
            image.setPixelColor(x, y, recolored);
        }
    }

    QIcon resultIcon;
    const QList<int> sizes = {20, 24, 32, 48};
    for (const int size : sizes) {
        resultIcon.addPixmap(QPixmap::fromImage(image).scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    return resultIcon;
}

} // namespace

namespace ui::shell {

NavigationController::NavigationController(QListWidget* navigationList, QWidget* navigationPane, QToolButton* navigationToggleButton)
    : navigationList_(navigationList),
      navigationPane_(navigationPane),
      navigationToggleButton_(navigationToggleButton),
      expandedWidth_(app::constants::Values::Ui::kNavigationExpandedWidth),
      collapsedWidth_(app::constants::Values::Ui::kNavigationCollapsedWidth) {}

void NavigationController::initialize(const QStringList& titles) {
    if (!navigationList_) {
        return;
    }

    navigationList_->clear();
    const int itemCount = qMin(titles.size(), static_cast<int>(std::size(kNavigationItems)));
    for (int i = 0; i < itemCount; ++i) {
        const QIcon icon = buildNavigationIcon(QString::fromLatin1(kNavigationItems[i].iconPath),
                                               QColor(QString::fromLatin1(kNavigationItems[i].accentColor)));
        auto* item = new QListWidgetItem(icon.isNull() ? navigationList_->style()->standardIcon(QStyle::SP_FileIcon) : icon,
                                         titles[i]);
        item->setToolTip(titles[i]);
        navigationList_->addItem(item);
    }

    recalculateExpandedWidth();
    setCollapsed(collapsed_);
}

void NavigationController::bindToStack(QStackedWidget* stackedWidget, const std::vector<int>& pageIndexByNavigationRow) {
    if (!navigationList_ || !stackedWidget) {
        return;
    }

    QObject::connect(navigationList_, &QListWidget::currentRowChanged, stackedWidget,
                     [stackedWidget, pageIndexByNavigationRow](int row) {
                         if (row < 0 || row >= static_cast<int>(pageIndexByNavigationRow.size())) {
                             return;
                         }
                         stackedWidget->setCurrentIndex(pageIndexByNavigationRow[static_cast<std::size_t>(row)]);
                     });
    navigationList_->setCurrentRow(0);
}

void NavigationController::setCollapsed(bool collapsed) {
    collapsed_ = collapsed;
    if (navigationPane_) {
        navigationPane_->setFixedWidth(collapsed_ ? collapsedWidth_ : expandedWidth_);
    }
    if (navigationList_) {
        for (int i = 0; i < navigationList_->count(); ++i) {
            auto* item = navigationList_->item(i);
            item->setText(collapsed_ ? QString() : item->toolTip());
            item->setTextAlignment(collapsed_ ? Qt::AlignCenter : (Qt::AlignLeft | Qt::AlignVCenter));
        }
    }
    updateToggleUi(QObject::tr("Expand Navigation"), QObject::tr("Collapse Navigation"));
}

bool NavigationController::isCollapsed() const {
    return collapsed_;
}

void NavigationController::retranslate(const QStringList& titles,
                                       const QString& expandNavigationText,
                                       const QString& collapseNavigationText) {
    if (!navigationList_) {
        return;
    }

    const int itemCount = qMin(titles.size(), navigationList_->count());
    for (int i = 0; i < itemCount; ++i) {
        auto* item = navigationList_->item(i);
        item->setToolTip(titles[i]);
        item->setText(collapsed_ ? QString() : titles[i]);
    }

    recalculateExpandedWidth();
    if (navigationPane_) {
        navigationPane_->setFixedWidth(collapsed_ ? collapsedWidth_ : expandedWidth_);
    }
    updateToggleUi(expandNavigationText, collapseNavigationText);
}

void NavigationController::updateToggleUi(const QString& expandNavigationText, const QString& collapseNavigationText) {
    if (!navigationToggleButton_) {
        return;
    }
    navigationToggleButton_->setText(collapsed_ ? QStringLiteral("≫") : QStringLiteral("≪"));
    navigationToggleButton_->setToolTip(collapsed_ ? expandNavigationText : collapseNavigationText);
}

void NavigationController::recalculateExpandedWidth() {
    if (!navigationList_) {
        expandedWidth_ = app::constants::Values::Ui::kNavigationExpandedWidth;
        return;
    }

    const QFontMetrics metrics(navigationList_->fontMetrics());
    int maxTextWidth = 0;
    for (int i = 0; i < navigationList_->count(); ++i) {
        maxTextWidth = qMax(maxTextWidth, metrics.horizontalAdvance(navigationList_->item(i)->toolTip()));
    }
    expandedWidth_ = qMax(app::constants::Values::Ui::kNavigationExpandedWidth, maxTextWidth + 60);
}

} // namespace ui::shell
