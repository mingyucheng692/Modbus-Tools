#include "WaveformWidget.h"
#include <QVBoxLayout>
#include <QDateTime>

WaveformWidget::WaveformWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    chart_ = new QChart();
    chart_->setTitle("Register Monitor");
    chart_->setTheme(QChart::ChartThemeDark);
    
    series_ = new QLineSeries();
    chart_->addSeries(series_);
    
    axisX_ = new QValueAxis();
    axisX_->setTitleText("Time (s)");
    axisX_->setLabelFormat("%.1f");
    chart_->addAxis(axisX_, Qt::AlignBottom);
    series_->attachAxis(axisX_);
    
    axisY_ = new QValueAxis();
    axisY_->setTitleText("Value");
    chart_->addAxis(axisY_, Qt::AlignLeft);
    series_->attachAxis(axisY_);
    
    QChartView* chartView = new QChartView(chart_);
    chartView->setRenderHint(QPainter::Antialiasing);
    
    layout->addWidget(chartView);
    
    startTime_ = QDateTime::currentMSecsSinceEpoch();
    axisX_->setRange(0, 10);
    axisY_->setRange(0, 100);
}

void WaveformWidget::setMonitoredAddress(int address) {
    monitoredAddress_ = address;
    series_->clear();
    startTime_ = QDateTime::currentMSecsSinceEpoch();
    chart_->setTitle(QString("Monitor: Register %1").arg(address));
}

void WaveformWidget::clear() {
    series_->clear();
}

void WaveformWidget::addValue(int address, double value) {
    if (address != monitoredAddress_) return;
    
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    double t = (now - startTime_) / 1000.0;
    
    series_->append(t, value);
    
    if (t > axisX_->max()) {
        axisX_->setRange(axisX_->min() + 1, axisX_->max() + 1);
    }
    
    if (value > axisY_->max()) {
        axisY_->setMax(value * 1.2);
    }
    if (value < axisY_->min()) {
        axisY_->setMin(value * 0.8);
    }
}
