#pragma once
#include <QObject>
#include <QTranslator>
#include <QApplication>
#include <spdlog/spdlog.h>

class LanguageManager : public QObject {
    Q_OBJECT
public:
    static LanguageManager& instance();

    void loadLanguage(const QString& langCode);

private:
    LanguageManager() = default;
    QTranslator* translator_ = nullptr;
};
