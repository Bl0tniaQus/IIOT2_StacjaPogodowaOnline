/********************************************************************************
** Form generated from reading UI file 'desktop_app.ui'
**
** Created by: Qt User Interface Compiler version 5.15.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DESKTOP_APP_H
#define UI_DESKTOP_APP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Desktop_App
{
public:
    QWidget *centralwidget;
    QLabel *xdd;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(321, 107);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        xdd = new QLabel(centralwidget);
        xdd->setObjectName(QString::fromUtf8("xdd"));
        xdd->setGeometry(QRect(100, 0, 121, 41));
        xdd->setAlignment(Qt::AlignCenter);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 321, 30));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("Desktop_App", "MainWindow", nullptr));
        xdd->setText(QCoreApplication::translate("Desktop_App", "testowy tekst", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Desktop_App: public Ui_Desktop_App {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DESKTOP_APP_H
