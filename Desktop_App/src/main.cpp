#include "desktop_app.h"
#include <QApplication>
#include <mqtt/async_client.h>
#include <string>
#include <pthread.h>
int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    Desktop_App w;
    w.show();

    return app.exec();
}

