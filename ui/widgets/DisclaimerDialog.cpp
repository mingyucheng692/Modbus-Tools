#include "DisclaimerDialog.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QDesktopServices>
#include <QLocale>
#include <QUrl>

namespace ui::widgets {

namespace {
QString systemDisclaimerLocale() {
    QLocale locale = QLocale::system();
    if (locale.language() == QLocale::Chinese) {
        if (locale.script() == QLocale::TraditionalChineseScript || 
            locale.country() == QLocale::Taiwan || 
            locale.country() == QLocale::HongKong || 
            locale.country() == QLocale::Macau) {
            return QStringLiteral("zh_TW");
        }
        return QStringLiteral("zh_CN");
    }
    return QStringLiteral("en_US");
}
}

DisclaimerDialog::DisclaimerDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
    applyLanguage(systemDisclaimerLocale());
}

void DisclaimerDialog::setupUi() {
    setWindowTitle("⚠️ WARRING");
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

void DisclaimerDialog::applyLanguage(const QString& lang) {
    currentLang_ = lang;
    langEnButton_->setText("English");
    langZhCnButton_->setText("简体中文");
    langZhTwButton_->setText("繁體中文");

    langEnButton_->setEnabled(lang != "en_US");
    langZhCnButton_->setEnabled(lang != "zh_CN");
    langZhTwButton_->setEnabled(lang != "zh_TW");

    if (lang == "zh_CN") {
        languageLabel_->setText("语言切换：");
        contentLabel_->setText(
            "本工具按\"原样\"提供，仅供调试和学习使用。\n\n"
            "执行不当操作可能导致设备损坏或数据丢失。\n\n"
            "您需自行承担所有操作风险。"
        );
        detailButton_->setText("📄 查看详情");
        acceptButton_->setText("✅ 同意");
        exitButton_->setText("❌ 退出");
    } else if (lang == "zh_TW") {
        languageLabel_->setText("語言切換：");
        contentLabel_->setText(
            "本工具按「原樣」提供，僅供調試和學習使用。\n\n"
            "執行不當操作可能導致設備損壞或數據丟失。\n\n"
            "您需自行承擔所有操作風險。"
        );
        detailButton_->setText("📄 查看詳情");
        acceptButton_->setText("✅ 同意");
        exitButton_->setText("❌ 退出");
    } else {
        languageLabel_->setText("Language:");
        contentLabel_->setText(
            "This tool is provided \"AS IS\" for testing and analysis only.\n\n"
            "Improper operations may cause equipment damage or data loss.\n\n"
            "You are solely responsible for all operations."
        );
        detailButton_->setText("📄 Learn more");
        acceptButton_->setText("✅ Agree");
        exitButton_->setText("❌ Exit");
    }
}

}
