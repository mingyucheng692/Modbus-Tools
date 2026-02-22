#pragma once

#include <QMainWindow>
#include <QTranslator>

class QStackedWidget;
class QListWidget;
class QAction;
class QActionGroup;
class QMenu;
class QEvent;

namespace ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void setupUi();
    void createNavigation();
    void setupLanguageMenu();
    void retranslateUi();
    void applyLanguage(const QString& locale);
    void changeEvent(QEvent* event) override;

    QListWidget* navigationList_ = nullptr;
    QStackedWidget* stackedWidget_ = nullptr;
    QMenu* languageMenu_ = nullptr;
    QActionGroup* languageActionGroup_ = nullptr;
    QAction* langEnAction_ = nullptr;
    QAction* langZhCnAction_ = nullptr;
    QAction* langZhTwAction_ = nullptr;
    QTranslator qtTranslator_;
    QTranslator appTranslator_;
    QString currentLocale_ = "en_US";
};

} // namespace ui
