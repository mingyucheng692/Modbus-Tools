// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "DisclaimerDialog.h"
#include <QEvent>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QDesktopServices>
#include <QLocale>
#include <QTranslator>
#include <QUrl>

namespace ui::widgets {

namespace {
constexpr char kDisclaimerContext[] = "ui::widgets::DisclaimerDialog";
constexpr char kLanguageLabelSource[] = QT_TRANSLATE_NOOP("ui::widgets::DisclaimerDialog", "Language:");
constexpr char kDisclaimerBodySource[] = QT_TRANSLATE_NOOP(
    "ui::widgets::DisclaimerDialog",
    "This tool is provided \"AS IS\" for testing and analysis only.\n\n"
    "Improper operations may cause equipment damage or data loss.\n\n"
    "You are solely responsible for all operations.");
constexpr char kLearnMoreSource[] = QT_TRANSLATE_NOOP("ui::widgets::DisclaimerDialog", "📄 Learn more");
constexpr char kAgreeSource[] = QT_TRANSLATE_NOOP("ui::widgets::DisclaimerDialog", "✅ Agree");
constexpr char kExitSource[] = QT_TRANSLATE_NOOP("ui::widgets::DisclaimerDialog", "❌ Exit");

QString systemDisclaimerLocale() {
    QLocale locale = QLocale::system();
    if (locale.language() == QLocale::Chinese) {
        if (locale.script() == QLocale::TraditionalChineseScript ||
            locale.territory() == QLocale::Taiwan ||
            locale.territory() == QLocale::HongKong ||
            locale.territory() == QLocale::Macau) {
            return QStringLiteral("zh_TW");
        }
        return QStringLiteral("zh_CN");
    }
    return QStringLiteral("en_US");
}

QString translateWithLocale(const QTranslator& translator, bool useTranslator, const char* sourceText) {
    if (useTranslator) {
        const QString translated = translator.translate(kDisclaimerContext, sourceText);
        if (!translated.isEmpty()) {
            return translated;
        }
    }
    return QString::fromUtf8(sourceText);
}
} // namespace

DisclaimerDialog::DisclaimerDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
    retranslateUi();
    applyLanguage(systemDisclaimerLocale());
}

void DisclaimerDialog::setupUi() {
    setWindowTitle("⚠️ WARNING");
    setMinimumWidth(560);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(12);

    titleLabel_ = new QLabel(this);
    titleLabel_->setText("⚠️ Disclaimer / 免责声明 / 免責聲明");
    titleLabel_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    layout->addWidget(titleLabel_);

    auto langRow = new QHBoxLayout();
    languageLabel_ = new QLabel(this);
    langRow->addWidget(languageLabel_);

    langEnButton_ = new QPushButton(this);
    langEnButton_->setFlat(true);
    langEnButton_->setCursor(Qt::PointingHandCursor);
    langRow->addWidget(langEnButton_);

    auto separator1 = new QLabel("|", this);
    langRow->addWidget(separator1);

    langZhCnButton_ = new QPushButton(this);
    langZhCnButton_->setFlat(true);
    langZhCnButton_->setCursor(Qt::PointingHandCursor);
    langRow->addWidget(langZhCnButton_);

    auto separator2 = new QLabel("|", this);
    langRow->addWidget(separator2);

    langZhTwButton_ = new QPushButton(this);
    langZhTwButton_->setFlat(true);
    langZhTwButton_->setCursor(Qt::PointingHandCursor);
    langRow->addWidget(langZhTwButton_);

    langRow->addStretch(1);
    layout->addLayout(langRow);

    separatorLine_ = new QFrame(this);
    separatorLine_->setFrameShape(QFrame::HLine);
    separatorLine_->setFrameShadow(QFrame::Sunken);
    layout->addWidget(separatorLine_);

    contentLabel_ = new QLabel(this);
    contentLabel_->setWordWrap(true);
    contentLabel_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(contentLabel_);

    detailButton_ = new QPushButton(this);
    detailButton_->setFlat(true);
    detailButton_->setCursor(Qt::PointingHandCursor);
    layout->addWidget(detailButton_, 0, Qt::AlignLeft);

    auto buttonsRow = new QHBoxLayout();
    buttonsRow->addStretch(1);
    acceptButton_ = new QPushButton(this);
    exitButton_ = new QPushButton(this);
    buttonsRow->addWidget(acceptButton_);
    buttonsRow->addWidget(exitButton_);
    buttonsRow->addStretch(1);
    layout->addLayout(buttonsRow);

    connect(langEnButton_, &QPushButton::clicked, this, [this]() { applyLanguage("en_US"); });
    connect(langZhCnButton_, &QPushButton::clicked, this, [this]() { applyLanguage("zh_CN"); });
    connect(langZhTwButton_, &QPushButton::clicked, this, [this]() { applyLanguage("zh_TW"); });

    connect(detailButton_, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://github.com/mingyucheng692/Modbus-Tools"));
    });
    connect(acceptButton_, &QPushButton::clicked, this, &QDialog::accept);
    connect(exitButton_, &QPushButton::clicked, this, &QDialog::reject);
}

void DisclaimerDialog::retranslateUi() {
    languageLabel_->setText(tr(kLanguageLabelSource));
    contentLabel_->setText(tr(kDisclaimerBodySource));
    detailButton_->setText(tr(kLearnMoreSource));
    acceptButton_->setText(tr(kAgreeSource));
    exitButton_->setText(tr(kExitSource));
}

void DisclaimerDialog::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QDialog::changeEvent(event);
}

void DisclaimerDialog::applyLanguage(const QString& lang) {
    currentLang_ = lang;
    langEnButton_->setText(QStringLiteral("English"));
    langZhCnButton_->setText(QStringLiteral("简体中文"));
    langZhTwButton_->setText(QStringLiteral("繁體中文"));

    langEnButton_->setEnabled(lang != QStringLiteral("en_US"));
    langZhCnButton_->setEnabled(lang != QStringLiteral("zh_CN"));
    langZhTwButton_->setEnabled(lang != QStringLiteral("zh_TW"));

    QTranslator translator;
    bool loaded = false;
    if (lang == QStringLiteral("zh_CN") || lang == QStringLiteral("zh_TW")) {
        loaded = translator.load(QStringLiteral(":/i18n/Modbus-Tools_%1.qm").arg(lang));
    }

    languageLabel_->setText(translateWithLocale(translator, loaded, kLanguageLabelSource));
    contentLabel_->setText(translateWithLocale(translator, loaded, kDisclaimerBodySource));
    detailButton_->setText(translateWithLocale(translator, loaded, kLearnMoreSource));
    acceptButton_->setText(translateWithLocale(translator, loaded, kAgreeSource));
    exitButton_->setText(translateWithLocale(translator, loaded, kExitSource));
}

} // namespace ui::widgets
