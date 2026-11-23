// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QWidget>

class QTextEdit;
class QLabel;
class QRadioButton;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QPushButton;
class QProgressBar;
class QEvent;
class QString;

namespace infra::config {
class ISettingsService;
}

namespace ui::widgets {

class GenericInputWidget : public QWidget {
    Q_OBJECT

public:
    explicit GenericInputWidget(infra::config::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~GenericInputWidget() override;

    void setSettingsGroup(const QString& group);
    [[nodiscard]] QByteArray getData() const;
    [[nodiscard]] bool tryBuildPayload(QByteArray* out, QString* errorOut = nullptr) const;
    void setSendControlsEnabled(bool enabled, const QString& disabledTooltip = {});
    void setAutoSendEnabled(bool enabled);
    [[nodiscard]] bool isAutoSendEnabled() const;

signals:
    void sendRequested(const QByteArray& data);
    void autoSendToggled(bool checked, int intervalMs);
    void autoSendIntervalChanged(int intervalMs);
    void validationFailed(const QString& reason);

private slots:
    void onSendClicked();
    void onAutoSendToggled(bool checked);
    void onFormatChanged();
    void onHistoryActivated(int index);

private:
    void setupUi();
    void loadSettings();
    void saveSettings();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    void addToHistory(const QString& text);
    void saveHistory();
    void loadHistory();
    bool eventFilter(QObject* watched, QEvent* event) override;

    QTextEdit* inputEdit_ = nullptr;
    QLabel* errorLabel_ = nullptr;
    QRadioButton* hexRadio_ = nullptr;
    QRadioButton* asciiRadio_ = nullptr;
    QCheckBox* autoSendCheck_ = nullptr;
    QSpinBox* intervalSpin_ = nullptr;
    QPushButton* sendBtn_ = nullptr;
    QComboBox* lineEndingCombo_ = nullptr;
    QComboBox* encodingCombo_ = nullptr;
    QComboBox* sendHistoryCombo_ = nullptr;

    bool sendControlsEnabled_ = true;
    QString disabledTooltip_;
    QString settingsGroup_;
    infra::config::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::widgets
