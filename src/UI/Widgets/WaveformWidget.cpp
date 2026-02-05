#include "WaveformWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>

WaveformWidget::WaveformWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Control Bar
    QHBoxLayout* ctrlLayout = new QHBoxLayout();
    ctrlLayout->addWidget(new QLabel("Monitor Register:"));
    
    addrSpin_ = new QSpinBox(this);
    addrSpin_->setRange(0, 65535);
    addrSpin_->setValue(0);
    ctrlLayout->addWidget(addrSpin_);
    
    setBtn_ = new QPushButton("Set", this);
    ctrlLayout->addWidget(setBtn_);
    
    ctrlLayout->addStretch();
    layout->addLayout(ctrlLayout);
    
    // Chart
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
    
    connect(setBtn_, &QPushButton::clicked, this, &WaveformWidget::onSetAddressClicked);
}

void WaveformWidget::onSetAddressClicked() {
    setMonitoredAddress(addrSpin_->value());
}

void WaveformWidget::setMonitoredAddress(int address) {
    monitoredAddress_ = address;
    series_->clear();
    startTime_ = QDateTime::currentMSecsSinceEpoch();
    chart_->setTitle(QString("Monitor: Register %1").arg(address));
    addrSpin_->setValue(address); // Sync UI if set programmatically
}

void WaveformWidget::clear() {
    series_->clear();
}

void WaveformWidget::addValue(int address, double value) {
    // If not monitoring anything, or address mismatch
    if (monitoredAddress_ == -1) return; 
    
    // Simple logic: if address matches, plot it.
    // If user hasn't set an address yet (monitoredAddress_ == -1), we could auto-set?
    // Let's stick to explicit set for now to avoid jumping.
    
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
