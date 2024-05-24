#ifndef DESKTOP_APP_H
#define DESKTOP_APP_H

#include <QMainWindow>
#include <QScopedPointer>
#include <QThread>
#include <QtCharts>
#include "mqtt_controller.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;
namespace Ui {
class Desktop_App;
}

class Desktop_App : public QMainWindow
{
    Q_OBJECT

public:
    explicit Desktop_App(QWidget *parent = nullptr);
    ~Desktop_App() override;
    void drawTempChart(std::vector<short> h, std::vector<float> t, int min, int max);
public slots:
    void receiveMqttMessage(QString message);
signals:
    void startMqtt();
private:
    QScopedPointer<Ui::Desktop_App> m_ui;
    Mqtt_Controller mqttController;
    QThread mqttThread;
};

#endif // DESKTOP_APP_H
