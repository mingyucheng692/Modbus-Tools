#pragma once

#include <QString>

namespace ui::application {

class IMainWindowPresenter {
public:
    virtual ~IMainWindowPresenter() = default;

    virtual void initialize() = 0;
    virtual void onNavigationToggleRequested() = 0;
    virtual void onModbusSettingsRequested() = 0;
    virtual void onUpdateSettingsRequested() = 0;
    virtual void onAboutRequested() = 0;
    virtual void onCheckForUpdatesRequested() = 0;
    virtual void onLanguageSelected(const QString& locale) = 0;
    virtual void onCloseRequested() = 0;
};

} // namespace ui::application
