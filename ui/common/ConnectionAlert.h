#pragma once

#include <QObject>
#include <QMessageBox>

class QWidget;

namespace ui::common {

class ConnectionAlert {
    Q_DECLARE_TR_FUNCTIONS(ConnectionAlert)
public:
    static void showNotConnected(QWidget* parent) {
        QMessageBox::warning(parent, tr("Not Connected"), tr("Please connect first."));
    }
};

} // namespace ui::common
