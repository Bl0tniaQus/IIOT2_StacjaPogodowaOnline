#ifndef MQTT_CONTROLLER_H
#define MQTT_CONTROLLER_H
#include <QObject>
#include <mqtt/async_client.h>

class Mqtt_Controller : public QObject
{
    Q_OBJECT
public:
    Mqtt_Controller();
    ~Mqtt_Controller();
public slots:
    void mqttClient();
signals:
    void sendMqttMessage(QString message);
private:
    std::string SERVER_ADDRESS = "broker.mqttdashboard.com";
    std::string CLIENT_ID =  "pirtesterxd";
    std::string TOPIC = "pir/test/xd";
    const int  QOS = 1;

};

#endif
