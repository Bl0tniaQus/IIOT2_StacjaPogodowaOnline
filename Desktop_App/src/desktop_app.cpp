#include "desktop_app.h"
#include "ui_desktop_app.h"

Desktop_App::Desktop_App(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::Desktop_App)
{
    m_ui->setupUi(this);
    connect(this, &Desktop_App::startMqtt, &mqttController, &Mqtt_Controller::mqttClient);
    connect(&mqttController, &Mqtt_Controller::sendMqttMessage, this, &Desktop_App::receiveMqttMessage);
    mqttController.moveToThread(&mqttThread);
    mqttThread.start();


    emit startMqtt();
}

Desktop_App::~Desktop_App() = default;
void Desktop_App::receiveMqttMessage(QString message)
{
    QLabel* lbl = findChild<QLabel*>("xdd");
    lbl->setText(message);
}
