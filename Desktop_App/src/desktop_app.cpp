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
    connect(findChild<QPushButton*>("reconnectButton"), &QPushButton::released, this, &Desktop_App::startMqttThread);
    connect(&mqttController, &Mqtt_Controller::clientDisconnected, this, &Desktop_App::startMqttThread);
    connect(&mqttController, &Mqtt_Controller::clientConnected, this, &Desktop_App::connectionLabelTrue);
    startMqttThread();
}
void Desktop_App::startMqttThread()
{
    QLabel* connectionLabel = findChild<QLabel*>("connectionLabel");
    connectionLabel->setText("disconnected");
    mqttThread.exit();
    connect(this, &Desktop_App::startMqtt, &mqttController, &Mqtt_Controller::mqttClient);
    connect(&mqttController, &Mqtt_Controller::sendMqttMessage, this, &Desktop_App::receiveMqttMessage);
    mqttController.moveToThread(&mqttThread);
    mqttThread.start();
    emit startMqtt();
}
Desktop_App::~Desktop_App() = default;
void Desktop_App::connectionLabelTrue()
{
    QLabel* connectionLabel = findChild<QLabel*>("connectionLabel");
    connectionLabel->setText("connected");
}

void Desktop_App::receiveMqttMessage(QString message)
{
    json jsonMsg = json::parse(message.toStdString());

    int temp_LB,temp_UB,pres_LB,pres_UB,hum_LB,hum_UB,pres,hum;
    float temp;
    try {temp_LB = int(jsonMsg["temp_LB"]);}
    catch (...) {temp_LB = -9999; findChild<QLabel*>("tempBounds")->setText("<null;null>");}
    try {temp = float(jsonMsg["temp"]); findChild<QLabel*>("temp")->setText(QString::number(temp));}
    catch (...) {temp = 9999; findChild<QLabel*>("temp")->setText("null");}
    try {
        temp_UB = int(jsonMsg["temp_UB"]);
        findChild<QLabel*>("tempBounds")->setText("<"+QString::number(temp_LB)+";"+QString::number(temp_UB)+">");
    }
    catch (...) {temp_UB = 9999; findChild<QLabel*>("tempBounds")->setText("<null;null>");}

    try {pres_LB = int(jsonMsg["pres_LB"]);}
    catch (...) {pres_LB = -9999; findChild<QLabel*>("presBounds")->setText("<null;null>");}
    try {pres = int(jsonMsg["pres"]); findChild<QLabel*>("pres")->setText(QString::number(pres));}
    catch (...) {pres = 9999; findChild<QLabel*>("pres")->setText("null");}
    try {
        pres_UB = int(jsonMsg["pres_UB"]);
        findChild<QLabel*>("presBounds")->setText("<"+QString::number(pres_LB)+";"+QString::number(pres_UB)+">");
    }
    catch (...) {pres_UB = 9999; findChild<QLabel*>("presBounds")->setText("<null;null>");}

    try {hum_LB = int(jsonMsg["hum_LB"]);}
    catch (...) {hum_LB = -9999; findChild<QLabel*>("humBounds")->setText("<null;null>");}
    try {hum = int(jsonMsg["hum"]); findChild<QLabel*>("hum")->setText(QString::number(hum));}
    catch (...) {hum = 9999; findChild<QLabel*>("hum")->setText("null");}
    try {
        hum_UB = int(jsonMsg["hum_UB"]);
        findChild<QLabel*>("humBounds")->setText("<"+QString::number(hum_LB)+";"+QString::number(hum_UB)+">");
    }
    catch (...) {hum_UB = 9999; findChild<QLabel*>("humBounds")->setText("<null;null>");}


    QFrame* tempFrame = findChild<QFrame*>("tempFrame");
    QFrame* presFrame = findChild<QFrame*>("presFrame");
    QFrame* humFrame = findChild<QFrame*>("humFrame");

    tempFrame->setStyleSheet("background-color:#1b1e20; border:6px solid black;");
    presFrame->setStyleSheet("background-color:#1b1e20; border:6px solid black;");
    humFrame->setStyleSheet("background-color:#1b1e20; border:6px solid black;");

    if ((temp > temp_UB && temp < temp_UB+2) || (temp <= temp_LB && temp >= temp_LB - 2))
    {
      tempFrame->setStyleSheet("background-color:#1b1e20; border:6px solid maroon;");
      }
    else if (temp > temp_UB + 2 || temp < temp_LB - 2) {
      tempFrame->setStyleSheet("background-color:maroon; border:6px solid black;");
    }

    if ((pres > pres_UB && pres < pres_UB+5) || (pres <= pres_LB && pres >= pres_LB - 5))
    {
      presFrame->setStyleSheet("background-color:#1b1e20; border:6px solid maroon;");
      }
    else if (pres > pres_UB + 5 || pres < pres_LB - 5) {
      presFrame->setStyleSheet("background-color:maroon; border:6px solid black;");
    }

    if ((hum > hum_UB && hum < hum_UB+2) || (hum <= hum_LB && hum >= hum_LB - 2))
    {
      humFrame->setStyleSheet("background-color:#1b1e20; border:6px solid maroon;");
      }
    else if (hum > hum_UB + 2 || hum < hum_LB - 2) {
      humFrame->setStyleSheet("background-color:maroon; border:6px solid black;");
    }


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
    drawPresChart(hours,pressures, pres_LB, pres_UB);
    drawHumChart(hours,humidities, hum_LB, hum_UB);
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
void Desktop_App::drawHumChart(std::vector<short> h, std::vector<short> t, int lb, int ub)
{
    QLineSeries *series = new QLineSeries();
    QLineSeries *seriesLB = new QLineSeries();
    QLineSeries *seriesUB = new QLineSeries();
    QLineSeries *lastLB = new QLineSeries();
    QLineSeries *lastUB = new QLineSeries();
    short n = h.size();
    float minH,maxH;
    for (int i=0; i<n;i++)
    {
        if (i==0) {minH = t[i]; maxH = t[i];}
        else {
            if (t[i]<minH) {minH = t[i];}
            if (t[i]>maxH) {maxH = t[i];}
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
    if (ub>maxH) {maxH = ub;}
    if (lb<minH) {minH = lb;}
    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(minH,maxH);
    axisY->setLabelFormat("%d");
    axisX->setRange(-(n-1),0);
    axisX->setLabelFormat("%d");
    axisX->setTickCount(7);
    axisX->setTitleText("Time [h]");
    axisY->setTitleText("Hum [%]");
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
    pen.setColor(Qt::blue);
    pen2.setWidth(2);
    pen2.setColor(Qt::red);
    lastLB->setPointLabelsVisible(true); lastUB->setPointLabelsVisible(true);
    lastLB->setPointLabelsColor(Qt::red); lastUB->setPointLabelsColor(Qt::red);
    lastLB->setPointLabelsFormat("@yPoint"); lastUB->setPointLabelsFormat("@yPoint");
    series->setPen(pen); seriesLB->setPen(pen2); seriesUB->setPen(pen2);
    lastLB->setPen(pen2); lastUB->setPen(pen2);
    chart->legend()->hide();
    chart->setTitle("Humidity");
    QFont font; font.setPixelSize(18);
    chart->setTitleFont(font);
    QChartView* wykres = findChild<QChartView*>("humChart");
    wykres->setChart(chart);
    wykres->setVisible(true);
}
void Desktop_App::drawPresChart(std::vector<short> h, std::vector<short> t, int lb, int ub)
{
    QLineSeries *series = new QLineSeries();
    QLineSeries *seriesLB = new QLineSeries();
    QLineSeries *seriesUB = new QLineSeries();
    QLineSeries *lastLB = new QLineSeries();
    QLineSeries *lastUB = new QLineSeries();
    short n = h.size();
    float minP,maxP;
    for (int i=0; i<n;i++)
    {
        if (i==0) {minP = t[i]; maxP = t[i];}
        else {
            if (t[i]<minP) {minP = t[i];}
            if (t[i]>maxP) {maxP = t[i];}
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
    if (ub>maxP) {maxP = ub;}
    if (lb<minP) {minP = lb;}
    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(minP,maxP);
    axisY->setLabelFormat("%d");
    axisX->setRange(-(n-1),0);
    axisX->setLabelFormat("%d");
    axisX->setTickCount(7);
    axisX->setTitleText("Time [h]");
    axisY->setTitleText("p [hPa]");
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
    pen.setColor(Qt::green);
    pen2.setWidth(2);
    pen2.setColor(Qt::red);
    lastLB->setPointLabelsVisible(true); lastUB->setPointLabelsVisible(true);
    lastLB->setPointLabelsColor(Qt::red); lastUB->setPointLabelsColor(Qt::red);
    lastLB->setPointLabelsFormat("@yPoint"); lastUB->setPointLabelsFormat("@yPoint");
    series->setPen(pen); seriesLB->setPen(pen2); seriesUB->setPen(pen2);
    lastLB->setPen(pen2); lastUB->setPen(pen2);
    chart->legend()->hide();
    chart->setTitle("Pressure");
    QFont font; font.setPixelSize(18);
    chart->setTitleFont(font);
    QChartView* wykres = findChild<QChartView*>("presChart");
    wykres->setChart(chart);
    wykres->setVisible(true);
}


