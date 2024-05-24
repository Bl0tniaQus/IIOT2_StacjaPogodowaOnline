#include "desktop_app.h"
#include "ui_desktop_app.h"

Desktop_App::Desktop_App(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::Desktop_App)
{
    m_ui->setupUi(this);
    findChild<QChartView*>("tempChart")->setVisible(false);
    findChild<QChartView*>("presChart")->setVisible(false);
    findChild<QChartView*>("humChart")->setVisible(false);
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
    catch (...) {temp_LB = -999; findChild<QLabel*>("temp_LB")->setText("null");}
    try {temp = float(jsonMsg["temp"]); findChild<QLabel*>("temp")->setText(QString::number(temp));}
    catch (...) {temp = -999; findChild<QLabel*>("temp")->setText("null");}
    try {temp_UB = int(jsonMsg["temp_UB"]); findChild<QLabel*>("temp_UB")->setText(QString::number(temp_UB));}
    catch (...) {temp_UB = -999; findChild<QLabel*>("temp_LB")->setText("null");}

    try {hum_LB = int(jsonMsg["hum_LB"]); findChild<QLabel*>("hum_LB")->setText(QString::number(hum_LB));}
    catch (...) {hum_LB = -999; findChild<QLabel*>("hum_LB")->setText("null");}
    try {hum = int(jsonMsg["hum"]); findChild<QLabel*>("hum")->setText(QString::number(hum));}
    catch (...) {hum = -999; findChild<QLabel*>("hum")->setText("null");}
    try {hum_UB = int(jsonMsg["hum_UB"]); findChild<QLabel*>("hum_UB")->setText(QString::number(hum_UB));}
    catch (...) {hum_UB = -999; findChild<QLabel*>("hum_LB")->setText("null");}

    try {pres_LB = int(jsonMsg["pres_LB"]); findChild<QLabel*>("pres_LB")->setText(QString::number(pres_LB));}
    catch (...) {pres_LB = -999; findChild<QLabel*>("pres_LB")->setText("null");}
    try {pres = float(jsonMsg["pres"]); findChild<QLabel*>("pres")->setText(QString::number(pres));}
    catch (...) {pres = -999; findChild<QLabel*>("pres")->setText("null");}
    try {pres_UB = int(jsonMsg["pres_UB"]); findChild<QLabel*>("pres_UB")->setText(QString::number(pres_UB));}
    catch (...) {pres_UB = -999; findChild<QLabel*>("pres_LB")->setText("null");}

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
    drawTempChart(hours,temps, temp_LB, temp_UB);
}
void Desktop_App::drawTempChart(std::vector< short int > h, std::vector< float > t, int lb, int ub)
{
    QLineSeries *series = new QLineSeries();
    QLineSeries *seriesLB = new QLineSeries();
    QLineSeries *seriesUB = new QLineSeries();
    QLineSeries *lastLB = new QLineSeries();
    QLineSeries *lastUB = new QLineSeries();
    short n = h.size();
    float minT,maxT;
    for (int i=0; i<n;i++)
    {
        if (i==0) {minT = t[i]; maxT = t[i];}
        else {
            if (t[i]<minT) {minT = t[i];}
            if (t[i]>maxT) {maxT = t[i];}
        }
        if (i==1)
        {
            lastLB->append(i-71,lb);
            lastUB->append(i-71,ub);
        }
        seriesLB->append(i-71,lb);
        seriesUB->append(i-71,ub);
        series->append(i-71,t[i]);
    }
    if (float(ub)>maxT) {maxT = float(ub);}
    if (float(lb)<minT) {minT = float(lb);}
    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(minT,maxT);
    axisY->setLabelFormat("%.1f");
    axisX->setRange(-(n-1),0);
    axisX->setLabelFormat("%d");
    axisX->setTickCount(7);
    axisX->setTitleText("Time [h]");
    axisY->setTitleText("T [°C]");
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->addSeries(seriesLB);
    chart->addSeries(seriesUB);
    chart->addSeries(lastLB);
    chart->addSeries(lastUB);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX); series->attachAxis(axisY);
    seriesLB->attachAxis(axisX); seriesLB->attachAxis(axisY);
    seriesUB->attachAxis(axisX); seriesUB->attachAxis(axisY);
    lastLB->attachAxis(axisX); lastLB->attachAxis(axisY);
    lastUB->attachAxis(axisX); lastUB->attachAxis(axisY);
    chart->setMargins(QMargins(0,0,0,0));
    QPen pen = series->pen();
    QPen pen2 = seriesLB->pen();
    pen.setWidth(4);
    pen.setColor(QColor::fromRgb(255,140,0));
    pen2.setWidth(2);
    pen2.setColor(Qt::red);
    lastLB->setPointLabelsVisible(true); lastUB->setPointLabelsVisible(true);
    lastLB->setPointLabelsColor(Qt::red); lastUB->setPointLabelsColor(Qt::red);
    lastLB->setPointLabelsFormat("@yPoint"); lastUB->setPointLabelsFormat("@yPoint");
    series->setPen(pen); seriesLB->setPen(pen2); seriesUB->setPen(pen2);
    lastLB->setPen(pen2); lastUB->setPen(pen2);
    chart->legend()->hide();
    chart->setTitle("Temperature");
    QFont font; font.setPixelSize(18);
    chart->setTitleFont(font);
    QChartView* wykres = findChild<QChartView*>("tempChart");
    wykres->setChart(chart);
    wykres->setVisible(true);
}

