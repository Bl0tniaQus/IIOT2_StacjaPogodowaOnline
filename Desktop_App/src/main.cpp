#include "desktop_app.h"
#include <QApplication>
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
    Desktop_App w;
    w.show();
    return app.exec();
}
//todo
//m5 owm jak starczy czasu
//m5 "api", remote configs
