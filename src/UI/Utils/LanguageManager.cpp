#include "LanguageManager.h"

LanguageManager& LanguageManager::instance() {
    static LanguageManager instance;
    return instance;
}

void LanguageManager::loadLanguage(const QString& langCode) {
    if (translator_) {
        qApp->removeTranslator(translator_);
        delete translator_;
        translator_ = nullptr;
    }

    if (langCode == "en_US") {
        spdlog::info("Language switched to English");
        return; // Default language
    }

    translator_ = new QTranslator(this);
    QString resourcePath = QString(":/translations/modbustools_%1.qm").arg(langCode);
    
    if (translator_->load(resourcePath)) {
        qApp->installTranslator(translator_);
        spdlog::info("Language switched to {}", langCode.toStdString());
    } else {
        spdlog::error("Failed to load translation: {}", resourcePath.toStdString());
        delete translator_;
        translator_ = nullptr;
    }
}
