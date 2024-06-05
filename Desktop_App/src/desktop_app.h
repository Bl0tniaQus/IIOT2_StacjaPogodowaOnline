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
    void drawTempChart(std::vector<short> h, std::vector<float> t, int lb, int ub);
    void drawHumChart(std::vector<short> h, std::vector<short> t, int lb, int ub);
    void drawPresChart(std::vector<short> h, std::vector<short> t, int lb, int ub);
    void requestTLBChange();
    void requestTUBChange();
    void requestPLBChange();
    void requestPUBChange();
    void requestHLBChange();
    void requestHUBChange();
public slots:
    void receiveMqttMessage(QString message);
    void startMqttThread();
    void connectionLabelTrue();
    void connectionLabelFalse();
signals:
    void startMqtt();
    void emitTLB(int val);
    void emitTUB(int val);
    void emitPLB(int val);
    void emitPUB(int val);
    void emitHLB(int val);
    void emitHUB(int val);
private:
    QScopedPointer<Ui::Desktop_App> m_ui;
    Mqtt_Controller mqttController;
    QThread mqttControllerThread;

};

#endif // DESKTOP_APP_H
