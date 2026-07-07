/**
 * @file ByteMonitorWidget.h
 * @brief Generic byte-level traffic monitor with HEX/ASCII display, statistics, and recording.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <QByteArray>
#include <QList>
#include <QColor>
#include <QString>
#include <QElapsedTimer>
#include "LogListModel.h"
#include "TrafficStats.h"
#include "LogRecorder.h"

class QListView;
class QCheckBox;
class QComboBox;
class QPushButton;
class QLabel;
class QButtonGroup;
class QTimer;
class QEvent;

namespace core::common {
class ISettingsService;
}

namespace ui::widgets {

struct PendingLine {
    QString text;
    QColor color;
};

class ByteMonitorWidget : public QWidget {
    Q_OBJECT

public:
    explicit ByteMonitorWidget(core::common::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~ByteMonitorWidget() override;

    void appendTx(const QByteArray& data);
    void appendRx(const QByteArray& data);
    void appendInfo(const QString& message);
    void appendError(const QString& message);
    void appendWarn(const QString& message);
    void clear();
    void appendMessage(bool isTx, const QByteArray& data);
    void appendMessageWithClient(bool isTx, const QByteArray& data, int clientId);
    void setSettingsGroup(const QString& group);

signals:
    void txDataReceived(QByteArray data);
    void rxDataReceived(QByteArray data);
    void infoMessageReceived(QString message);
    void errorMessageReceived(QString message);
    void warnMessageReceived(QString message);

private slots:
    void onTxData(QByteArray data);
    void onRxData(QByteArray data);
    void onInfoMessage(QString message);
    void onErrorMessage(QString message);
    void onWarnMessage(QString message);
    void onFlushPending();
    void onStatsTimer();
    void onDisplayModeChanged(int id);
    void onTimestampFormatChanged(int index);
    void onRecordToggled(bool checked);
    void onPauseToggled(bool checked);
    void onClearClicked();
    void onSaveClicked();
    void onCopyClicked();

private:
    enum class DisplayMode {
        Hex = 0,
        Ascii = 1
    };

    enum class TimestampFormat {
        Absolute = 0,
        Relative = 1,
        None = 2
    };

    void setupUi();
    void appendLogLine(const QString& text, const QColor& color);
    void flushPending();
    QString formatData(const QByteArray& data) const;
    QString formatTimestamp();
    void updateStatsDisplay();
    void rebuildDisplay();
    bool isPaused() const;
    bool isRecording() const;
    void loadSettings();
    void saveSettings();
    void retranslateUi();
    void changeEvent(QEvent* event) override;

    LogListModel* logModel_ = nullptr;
    TrafficStats stats_;
    LogRecorder recorder_;
    QElapsedTimer elapsedTimer_;
    qint64 lastAppendTimeMs_ = 0;

    QListView* logView_ = nullptr;
    QButtonGroup* displayGroup_ = nullptr;
    QComboBox* timestampCombo_ = nullptr;
    QCheckBox* showTxCheck_ = nullptr;
    QCheckBox* showRxCheck_ = nullptr;
    QCheckBox* autoScrollCheck_ = nullptr;
    QPushButton* pauseBtn_ = nullptr;
    QPushButton* clearBtn_ = nullptr;
    QPushButton* saveBtn_ = nullptr;
    QPushButton* recordBtn_ = nullptr;
    QLabel* txStatsLabel_ = nullptr;
    QLabel* rxStatsLabel_ = nullptr;
    QLabel* rateLabel_ = nullptr;
    QWidget* statsBar_ = nullptr;
    QTimer* flushTimer_ = nullptr;
    QTimer* statsTimer_ = nullptr;
    QList<PendingLine> pendingLines_;

    DisplayMode displayMode_ = DisplayMode::Hex;
    TimestampFormat timestampFormat_ = TimestampFormat::Absolute;
    bool paused_ = false;
    QString settingsGroup_;
    core::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::widgets