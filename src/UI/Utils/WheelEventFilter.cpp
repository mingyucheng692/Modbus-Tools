#include "WheelEventFilter.h"
#include <QEvent>
#include <QWidget>
#include <QAbstractSpinBox>
#include <QComboBox>

WheelEventFilter::WheelEventFilter(QObject* parent) : QObject(parent) {}

bool WheelEventFilter::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::Wheel) {
        if (qobject_cast<QAbstractSpinBox*>(obj) || qobject_cast<QComboBox*>(obj)) {
            if (!static_cast<QWidget*>(obj)->hasFocus()) {
                event->ignore();
                return true; // Filter out the event
            }
        }
    }
    return QObject::eventFilter(obj, event);
}
