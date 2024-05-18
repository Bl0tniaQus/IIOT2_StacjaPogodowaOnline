#include "desktop_app.h"
#include "mqtt_controller.h"
#include <QApplication>
#include <QThread>
#include <string>
#include <pthread.h>
#include <iostream>

std::string message = "";
//todo qthread, połączenie mqtt + gui


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
    Desktop_App w;
    w.show();
    return app.exec();
}

