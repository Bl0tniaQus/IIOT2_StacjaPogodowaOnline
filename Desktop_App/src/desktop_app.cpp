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

    int temp_LB,temp_UB,pres_LB,pres_UB,hum_LB,hum_UB,pres,hum;
    float temp;
    try {temp_LB = int(jsonMsg["temp_LB"]); findChild<QLabel*>("temp_LB")->setText(QString::number(temp_LB));}
    catch (...) {findChild<QLabel*>("temp_LB")->setText("null");}
    try {findChild<QLabel*>("temp")->setText(QString::number(float(jsonMsg["temp"])));}
    catch (...) {findChild<QLabel*>("temp")->setText("null");}
    try {findChild<QLabel*>("temp_UB")->setText(QString::number(int(jsonMsg["temp_UB"])));}
    catch (...) {findChild<QLabel*>("temp_UB")->setText("null");}
    try {findChild<QLabel*>("pres_LB")->setText(QString::number(int(jsonMsg["pres_LB"])));}
    catch (...) {findChild<QLabel*>("pres_LB")->setText("null");}
    try {findChild<QLabel*>("pres")->setText(QString::number(int(jsonMsg["pres"])));}
    catch (...) {findChild<QLabel*>("pres")->setText("null");}
    try {findChild<QLabel*>("pres_UB")->setText(QString::number(int(jsonMsg["pres_UB"])));}
    catch (...) {findChild<QLabel*>("pres_UB")->setText("null");}
    try {findChild<QLabel*>("hum_LB")->setText(QString::number(int(jsonMsg["hum_LB"])));}
    catch (...) {findChild<QLabel*>("hum_LB")->setText("null");}
    try {findChild<QLabel*>("hum")->setText(QString::number(int(jsonMsg["hum"])));}
    catch (...) {findChild<QLabel*>("hum")->setText("null");}
    try {findChild<QLabel*>("hum_UB")->setText(QString::number(int(jsonMsg["hum_UB"])));}
    catch (...) {findChild<QLabel*>("hum_UB")->setText("null");}

    std::vector<short> hours;
    std::vector<float> temps;
    std::vector<short> pressures;
    std::vector<short> humidities;
    try {hours = jsonMsg["hours"].get<std::vector<short>>();}
    catch (...) {hours = {};}
    try {temps = jsonMsg["temps"].get<std::vector<float>>();}
    catch (...) {temps = {};}
    try {pressures = jsonMsg["pressures"].get<std::vector<short>>();}
    catch (...) {pressures = {};}
    try {humidities = jsonMsg["hums"].get<std::vector<short>>();}
    catch (...) {humidities = {};}
    drawTempChart(hours,temps);
}
void Desktop_App::drawTempChart(std::vector<short> h, std::vector<float> t)
{
     //testowy wykres
    QLineSeries *series = new QLineSeries();
    short n = h.size();
    for (int i=0; i<n;i++)
    {

        series->append(i-71,t[i]);
    }
    QValueAxis *axisX= new QValueAxis();
    axisX->setRange(-(n-1),0);
    axisX->setLabelFormat("%d");
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->legend()->hide();
    chart->createDefaultAxes();
    chart->removeAxis(chart->axes(Qt::Horizontal)[0]);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->setMargins(QMargins(0,0,0,0));
    QChartView* wykres = findChild<QChartView*>("graphicsView");
    wykres->setChart(chart);
}

