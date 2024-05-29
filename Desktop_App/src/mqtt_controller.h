#ifndef MQTT_CONTROLLER_H
#define MQTT_CONTROLLER_H
#include <QObject>
#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
class Mqtt_Controller : public QObject
{
    Q_OBJECT
public:
    Mqtt_Controller();
    ~Mqtt_Controller();
    bool isConnected();
    QString checkJson(std::string jsonStr);
public slots:
    void mqttClient();
signals:
    void sendMqttMessage(QString message);
    void clientDisconnected();
    void clientConnected();
private:
    bool status;
    std::string SERVER_ADDRESS = "broker.mqttdashboard.com";
    std::string CLIENT_ID =  "pirtesterxd";
    std::string TOPIC = "pir/test/xd";
    const int  QOS = 1;

};

#endif
