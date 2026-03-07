#include "UpdateSettingsDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>

namespace ui::widgets {

UpdateSettingsDialog::UpdateSettingsDialog(const QString& currentFrequency, QWidget* parent)
    : QDialog(parent) {
    setupUi();
    applyFrequency(currentFrequency);
}

QString UpdateSettingsDialog::selectedFrequency() const {
    return frequencyCombo_->currentData().toString();
}

void UpdateSettingsDialog::setupUi() {
    setWindowTitle(tr("Update Settings"));
    auto layout = new QVBoxLayout(this);
    auto formLayout = new QFormLayout();

    frequencyCombo_ = new QComboBox(this);
    frequencyCombo_->addItem(tr("Every Startup"), "startup");
    frequencyCombo_->addItem(tr("Weekly"), "weekly");
    frequencyCombo_->addItem(tr("Monthly"), "monthly");
    frequencyCombo_->addItem(tr("Disable Update Check"), "off");
    formLayout->addRow(tr("Update Check Frequency:"), frequencyCombo_);

    layout->addLayout(formLayout);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
}

void UpdateSettingsDialog::applyFrequency(const QString& frequency) {
    const int targetIndex = frequencyCombo_->findData(frequency);
    frequencyCombo_->setCurrentIndex(targetIndex >= 0 ? targetIndex : 0);
}

} // namespace ui::widgets
