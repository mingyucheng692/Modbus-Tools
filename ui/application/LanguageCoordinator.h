#pragma once

#include <QString>
#include <QTranslator>

namespace core::common {
class SettingsController;
}

namespace ui::application {

class LanguageCoordinator final {
public:
    explicit LanguageCoordinator(core::common::SettingsController* settingsController);

    void initialize();
    void applyLanguage(const QString& locale);

    [[nodiscard]] QString currentLocale() const;
    [[nodiscard]] QString effectiveLocale() const;

private:
    static QString normalizedAppLocale(QString locale);
    static QString effectiveAppLocale(const QString& locale);
    static QString translationFileNameForLocale(const QString& locale);
    static bool loadAppTranslation(QTranslator& translator, const QString& qmFileName);

    core::common::SettingsController* settingsController_ = nullptr;
    QTranslator qtTranslator_;
    QTranslator appTranslator_;
    QString currentLocale_ = QStringLiteral("en_US");
    QString effectiveLocale_ = QStringLiteral("en_US");
};

} // namespace ui::application
