#pragma once

#include <QStringList>
#include <vector>

class QListWidget;
class QStackedWidget;
class QToolButton;
class QWidget;

namespace ui::shell {

class NavigationController final {
public:
    NavigationController(QListWidget* navigationList, QWidget* navigationPane, QToolButton* navigationToggleButton);

    void initialize(const QStringList& titles);
    void bindToStack(QStackedWidget* stackedWidget, const std::vector<int>& pageIndexByNavigationRow);
    void setCollapsed(bool collapsed);
    [[nodiscard]] bool isCollapsed() const;
    void retranslate(const QStringList& titles, const QString& expandNavigationText, const QString& collapseNavigationText);

private:
    void updateToggleUi(const QString& expandNavigationText, const QString& collapseNavigationText);
    void recalculateExpandedWidth();

    QListWidget* navigationList_ = nullptr;
    QWidget* navigationPane_ = nullptr;
    QToolButton* navigationToggleButton_ = nullptr;
    bool collapsed_ = false;
    int expandedWidth_ = 138;
    int collapsedWidth_ = 52;
};

} // namespace ui::shell
