#pragma once
#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QMap>

class WaveformWidget : public QWidget {
    Q_OBJECT
public:
    explicit WaveformWidget(QWidget* parent = nullptr);

public slots:
    void addValue(int address, double value);
    void clear();
    void setMonitoredAddress(int address);

private:
    QChart* chart_;
    QLineSeries* series_;
    QValueAxis* axisX_;
    QValueAxis* axisY_;
    
    int monitoredAddress_ = -1;
    qint64 startTime_ = 0;
};
