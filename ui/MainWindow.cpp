#include "MainWindow.h"
#include "views/tcp/TcpView.h"
#include "views/rtu/RtuView.h"
#include "views/generic_tcp/GenericTcpView.h"
#include "views/generic_serial/GenericSerialView.h"
#include "widgets/FrameAnalyzerWidget.h"
#include "common/Theme.h"
#include <QStackedWidget>
#include <QListWidget>
#include <QHBoxLayout>
#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QEvent>
#include <QTranslator>
#include <QLibraryInfo>
#include <spdlog/spdlog.h>

namespace ui {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUi();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi() {
    setWindowTitle(tr("Modbus Tools"));
    resize(1200, 800);

    // Central widget with HBox layout
    auto centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Navigation
    createNavigation();
    mainLayout->addWidget(navigationList_, 1);

    // Content Area
    stackedWidget_ = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget_, 5); // Ratio 1:5

    // Add Views
    stackedWidget_->addWidget(new views::tcp::TcpView(this));
    stackedWidget_->addWidget(new views::rtu::RtuView(this));
    stackedWidget_->addWidget(new views::generic_tcp::GenericTcpView(this));
    stackedWidget_->addWidget(new views::generic_serial::GenericSerialView(this));
    stackedWidget_->addWidget(new widgets::FrameAnalyzerWidget(this));

    // Connect Navigation
    connect(navigationList_, &QListWidget::currentRowChanged,
            stackedWidget_, &QStackedWidget::setCurrentIndex);
            
    // Set default view
    navigationList_->setCurrentRow(0);

    setupLanguageMenu();
    applyLanguage(currentLocale_);
}

void MainWindow::createNavigation() {
    navigationList_ = new QListWidget(this);
    navigationList_->addItem(tr("Modbus TCP"));
    navigationList_->addItem(tr("Modbus RTU"));
    navigationList_->addItem(tr("TCP Client"));
    navigationList_->addItem(tr("Serial Port"));
    navigationList_->addItem(tr("Frame Analyzer"));
    
    // Style the navigation list a bit
    // Use light theme by default for now
    navigationList_->setStyleSheet(common::Theme::getNavigationStyle(false));
}

void MainWindow::setupLanguageMenu() {
    languageMenu_ = menuBar()->addMenu(tr("Language"));
    languageActionGroup_ = new QActionGroup(this);
    languageActionGroup_->setExclusive(true);

    langEnAction_ = languageMenu_->addAction(tr("English (US)"));
    langEnAction_->setCheckable(true);
    langEnAction_->setData("en_US");
    languageActionGroup_->addAction(langEnAction_);

    langZhCnAction_ = languageMenu_->addAction(tr("简体中文"));
    langZhCnAction_->setCheckable(true);
    langZhCnAction_->setData("zh_CN");
    languageActionGroup_->addAction(langZhCnAction_);

    langZhTwAction_ = languageMenu_->addAction(tr("繁體中文"));
    langZhTwAction_->setCheckable(true);
    langZhTwAction_->setData("zh_TW");
    languageActionGroup_->addAction(langZhTwAction_);

    connect(languageActionGroup_, &QActionGroup::triggered, this, [this](QAction* action) {
        applyLanguage(action->data().toString());
    });
}

void MainWindow::applyLanguage(const QString& locale) {
    qApp->removeTranslator(&qtTranslator_);
    qApp->removeTranslator(&appTranslator_);

    currentLocale_ = locale;
    if (locale == "zh_CN") {
        const bool qtLoaded = qtTranslator_.load("qtbase_zh_CN", QLibraryInfo::path(QLibraryInfo::TranslationsPath));
        const bool appLoaded = appTranslator_.load(":/i18n/ModbusTools_zh_CN.qm");
        if (qtLoaded) {
            qApp->installTranslator(&qtTranslator_);
        }
        if (appLoaded) {
            qApp->installTranslator(&appTranslator_);
        }
    } else if (locale == "zh_TW") {
        const bool qtLoaded = qtTranslator_.load("qtbase_zh_TW", QLibraryInfo::path(QLibraryInfo::TranslationsPath));
        const bool appLoaded = appTranslator_.load(":/i18n/ModbusTools_zh_TW.qm");
        if (qtLoaded) {
            qApp->installTranslator(&qtTranslator_);
        }
        if (appLoaded) {
            qApp->installTranslator(&appTranslator_);
        }
    }

    retranslateUi();
}

void MainWindow::retranslateUi() {
    setWindowTitle(tr("Modbus Tools"));
    if (navigationList_ && navigationList_->count() >= 5) {
        navigationList_->item(0)->setText(tr("Modbus TCP"));
        navigationList_->item(1)->setText(tr("Modbus RTU"));
        navigationList_->item(2)->setText(tr("TCP Client"));
        navigationList_->item(3)->setText(tr("Serial Port"));
        navigationList_->item(4)->setText(tr("Frame Analyzer"));
    }

    if (languageMenu_) {
        languageMenu_->setTitle(tr("Language"));
    }
    if (langEnAction_) {
        langEnAction_->setText(tr("English (US)"));
        langEnAction_->setChecked(currentLocale_ == "en_US");
    }
    if (langZhCnAction_) {
        langZhCnAction_->setText(tr("简体中文"));
        langZhCnAction_->setChecked(currentLocale_ == "zh_CN");
    }
    if (langZhTwAction_) {
        langZhTwAction_->setText(tr("繁體中文"));
        langZhTwAction_->setChecked(currentLocale_ == "zh_TW");
    }
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

} // namespace ui
