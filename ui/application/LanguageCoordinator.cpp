#include "application/LanguageCoordinator.h"

#include "AppConstants.h"
#include "../core/common/SettingsController.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <spdlog/spdlog.h>

namespace ui::application {

LanguageCoordinator::LanguageCoordinator(core::common::SettingsController* settingsController)
    : settingsController_(settingsController) {
    Q_ASSERT(settingsController_);
}

void LanguageCoordinator::initialize() {
    applyLanguage(settingsController_->language());
}

void LanguageCoordinator::applyLanguage(const QString& locale) {
    qApp->removeTranslator(&qtTranslator_);
    qApp->removeTranslator(&appTranslator_);

    currentLocale_ = normalizedAppLocale(locale);
    effectiveLocale_ = effectiveAppLocale(currentLocale_);
    settingsController_->setLanguage(currentLocale_);

    if (effectiveLocale_ == app::constants::Values::App::kLocaleZhCn) {
        if (qtTranslator_.load(QStringLiteral("qtbase_zh_CN"), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
            qApp->installTranslator(&qtTranslator_);
        }
        if (loadAppTranslation(appTranslator_, translationFileNameForLocale(effectiveLocale_))) {
            qApp->installTranslator(&appTranslator_);
        }
        return;
    }

    if (effectiveLocale_ == app::constants::Values::App::kLocaleZhTw) {
        if (qtTranslator_.load(QStringLiteral("qtbase_zh_TW"), QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
            qApp->installTranslator(&qtTranslator_);
        }
        if (loadAppTranslation(appTranslator_, translationFileNameForLocale(effectiveLocale_))) {
            qApp->installTranslator(&appTranslator_);
        }
    }
}

QString LanguageCoordinator::currentLocale() const {
    return currentLocale_;
}

QString LanguageCoordinator::effectiveLocale() const {
    return effectiveLocale_;
}

QString LanguageCoordinator::normalizedAppLocale(QString locale) {
    locale = locale.trimmed();
    if (locale == QLatin1String(app::constants::Values::App::kLocaleEn) ||
        locale == QLatin1String(app::constants::Values::App::kLocaleZhCn) ||
        locale == QLatin1String(app::constants::Values::App::kLocaleZhTw) ||
        locale == QStringLiteral("system")) {
        return locale;
    }
    return QStringLiteral("system");
}

QString LanguageCoordinator::effectiveAppLocale(const QString& locale) {
    if (locale != QStringLiteral("system")) {
        return locale;
    }

    const QLocale systemLocale = QLocale::system();
    if (systemLocale.language() == QLocale::Chinese) {
        if (systemLocale.script() == QLocale::TraditionalChineseScript ||
            systemLocale.country() == QLocale::Taiwan ||
            systemLocale.country() == QLocale::HongKong ||
            systemLocale.country() == QLocale::Macau) {
            return QLatin1String(app::constants::Values::App::kLocaleZhTw);
        }
        return QLatin1String(app::constants::Values::App::kLocaleZhCn);
    }

    return QLatin1String(app::constants::Values::App::kLocaleEn);
}

QString LanguageCoordinator::translationFileNameForLocale(const QString& locale) {
    if (locale == QLatin1String(app::constants::Values::App::kLocaleZhCn)) {
        return QStringLiteral("Modbus-Tools_zh_CN.qm");
    }
    if (locale == QLatin1String(app::constants::Values::App::kLocaleZhTw)) {
        return QStringLiteral("Modbus-Tools_zh_TW.qm");
    }
    return {};
}

bool LanguageCoordinator::loadAppTranslation(QTranslator& translator, const QString& qmFileName) {
    if (qmFileName.isEmpty()) {
        return false;
    }

    const QString resourcePath = QStringLiteral(":/i18n/%1").arg(qmFileName);
    if (translator.load(resourcePath)) {
        spdlog::info("Loaded app translation from embedded resource: {}", resourcePath.toStdString());
        return true;
    }

    spdlog::warn("Failed to load app translation '{}' from embedded resource '{}'",
                 qmFileName.toStdString(),
                 resourcePath.toStdString());
    return false;
}

} // namespace ui::application
