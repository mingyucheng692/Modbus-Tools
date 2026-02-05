#include <QApplication>
#include <QMainWindow>
#include <spdlog/spdlog.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    spdlog::info("Modbus-Tools Started");
    spdlog::info("Qt Version: {}", qVersion());

    QMainWindow w;
    w.setWindowTitle("Modbus-Tools (Dev)");
    w.resize(800, 600);
    w.show();

    return a.exec();
}
