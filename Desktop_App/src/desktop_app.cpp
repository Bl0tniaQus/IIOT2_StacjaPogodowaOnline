#include "desktop_app.h"
#include "ui_desktop_app.h"

Desktop_App::Desktop_App(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::Desktop_App)
{
    m_ui->setupUi(this);
}

Desktop_App::~Desktop_App() = default;
