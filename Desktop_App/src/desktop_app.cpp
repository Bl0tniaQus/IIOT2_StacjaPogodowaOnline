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
    json jsonMsg = json::parse(message.toStdString());
    try {findChild<QLabel*>("temp_LB")->setText(QString::fromStdString(jsonMsg["temp_LB"]));}
    catch (json::type_error e) {findChild<QLabel*>("temp_LB")->setText("null");}
    try {findChild<QLabel*>("temp")->setText(QString::fromStdString(jsonMsg["temp"]));}
    catch (json::type_error e) {findChild<QLabel*>("temp")->setText("null");}
    try {findChild<QLabel*>("temp_UB")->setText(QString::fromStdString(jsonMsg["temp_UB"]));}
    catch (json::type_error e) {findChild<QLabel*>("temp_UB")->setText("null");}
    try {findChild<QLabel*>("pres_LB")->setText(QString::fromStdString(jsonMsg["pres_LB"]));}
    catch (json::type_error e) {findChild<QLabel*>("pres_LB")->setText("null");}
    try {findChild<QLabel*>("pres")->setText(QString::fromStdString(jsonMsg["pres"]));}
    catch (json::type_error e) {findChild<QLabel*>("pres")->setText("null");}
    try {findChild<QLabel*>("pres_UB")->setText(QString::fromStdString(jsonMsg["pres_UB"]));}
    catch (json::type_error e) {findChild<QLabel*>("pres_UB")->setText("null");}
    try {findChild<QLabel*>("hum_LB")->setText(QString::fromStdString(jsonMsg["hum_LB"]));}
    catch (json::type_error e) {findChild<QLabel*>("hum_LB")->setText("null");}
    try {findChild<QLabel*>("hum")->setText(QString::fromStdString(jsonMsg["hum"]));}
    catch (json::type_error e) {findChild<QLabel*>("hum")->setText("null");}
    try {findChild<QLabel*>("hum_UB")->setText(QString::fromStdString(jsonMsg["hum_UB"]));}
    catch (json::type_error e) {findChild<QLabel*>("hum_UB")->setText("null");}

}
