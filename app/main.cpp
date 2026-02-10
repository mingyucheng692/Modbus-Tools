#include <QApplication>
#include <QMainWindow>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QMainWindow window;
    window.setWindowTitle("Modbus Tools");
    window.resize(800, 600);
    window.show();

    qDebug() << "Hello Modbus-Tools initialized";

    return app.exec();
}
