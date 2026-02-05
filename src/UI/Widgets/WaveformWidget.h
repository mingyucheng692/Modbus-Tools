#pragma once
#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QMap>

#include <QSpinBox>
#include <QLabel>
#include <QPushButton>

class WaveformWidget : public QWidget {
    Q_OBJECT
public:
    explicit WaveformWidget(QWidget* parent = nullptr);

    int monitoredAddress() const { return monitoredAddress_; }

public slots:
    void addValue(int address, double value);
    void clear();
    void setMonitoredAddress(int address);

private slots:
    void onSetAddressClicked();

private:
    QChart* chart_;
    QLineSeries* series_;
    QValueAxis* axisX_;
    QValueAxis* axisY_;
    
    QSpinBox* addrSpin_;
    QPushButton* setBtn_;
    
    int monitoredAddress_ = -1;
    qint64 startTime_ = 0;
};
