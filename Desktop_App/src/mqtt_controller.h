#ifndef MQTT_CONTROLLER_H
#define MQTT_CONTROLLER_H
#include <QObject>
#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>
#include "subscriber.h"
#include "publisher.h"
#include <QThread>
class Mqtt_Controller : public QObject
{
    Q_OBJECT
public:
    Mqtt_Controller();
    ~Mqtt_Controller();
    bool isConnected();
    Publisher* getPublisher() {return &publisher;}
    Subscriber* getSubscriber() {return &subscriber;}
signals:
    void clientDisconnected();
    void clientConnected();
private:
    bool status;
    mqtt::async_client cli;
    Publisher publisher;
    Subscriber subscriber;
    static std::string SERVER_ADDRESS;
    static std::string CLIENT_ID;
    static std::string TOPIC;
    const int  QOS = 1;
    QThread mqttSubscribeThread;
    QThread mqttPublishThread;
};

#endif
