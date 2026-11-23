// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QWidget>
#include <QString>

class QLabel;
class QPushButton;
class QEvent;
class QHBoxLayout;
class QLayout;

namespace infra::config {
class ISettingsService;
}

namespace io {
enum class ChannelState;
}

namespace ui::widgets {
class CollapsibleSection;

// Connection display states shared by all connection widget implementations.
enum class DisplayState {
    Disconnected = 0,
    Connecting,
    Connected,
    Disconnecting,
    Listening,
    Bound
};

// Per-state display texts returned by getStateDisplayInfo().
struct StateDisplayInfo {
    QString buttonText;
    QString statusText;
    QString statusStyle;
};

// Role: Common base widget for connection panels (collapsible sections, settings persistence, display states).
// Thread-safety: Single-threaded (bound to GUI thread).
// Ownership: Owned by QObject parent hierarchy.
class BaseConnectionWidget : public QWidget {
    Q_OBJECT

public:
    using DisplayState = ui::widgets::DisplayState;

    explicit BaseConnectionWidget(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~BaseConnectionWidget() override;
    void setSettingsGroup(const QString& group);
    [[nodiscard]] bool isSectionExpanded() const;
    void setSectionExpanded(bool expanded);
    [[nodiscard]] virtual DisplayState connectedState() const { return DisplayState::Connected; }
    void applyChannelState(io::ChannelState state);

signals:
    void disconnectClicked();
    void sectionExpandedChanged(bool expanded);

public slots:
    virtual void setConnected(bool connected) = 0;
    void setDisplayState(DisplayState state);

protected:
    virtual void loadSettings() = 0;
    virtual void saveSettings() = 0;

    void createCommonWidgets(QWidget* parent);
    void setupCommonConnections();
    void loadCommonSettings();
    void saveCommonSettings();
    void retranslateCommonUi();
    QHBoxLayout* setupBaseUi();
    void setupBaseUiWithLayout(QLayout* contentLayout);
    void finishBaseUi(const QString& sectionSettingsKey = {});
    virtual void retranslateUi();

    void changeEvent(QEvent* event) override;

    // ---- Display-state template method ----
    // Skeleton: queries subclass for state-specific data, applies common
    // widgets, delegates input-widget enabling and protocol refresh.
    void applyDisplayState();
    [[nodiscard]] virtual StateDisplayInfo getStateDisplayInfo(DisplayState state) const = 0;
    virtual void applyInputWidgetsState(bool enabled) = 0;
    virtual void updateProtocolUi() {}
    [[nodiscard]] static bool inputsLocked(DisplayState state) noexcept;

    infra::config::ISettingsService* settingsService_ = nullptr;
    QString settingsGroup_;

    CollapsibleSection* section_ = nullptr;
    QPushButton* connectBtn_ = nullptr;
    QLabel* statusLabel_ = nullptr;

    DisplayState displayState_ = DisplayState::Disconnected;
};

} // namespace ui::widgets
