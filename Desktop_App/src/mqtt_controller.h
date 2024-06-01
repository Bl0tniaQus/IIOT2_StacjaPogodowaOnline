#ifndef MQTT_CONTROLLER_H
#define MQTT_CONTROLLER_H
#include <QObject>
#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>
#include "subscriber.h"
#include "publisher.h"
#include "connection_manager.h"
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
    Connection_Manager* getManager() {return &manager;}
    void checkConnection();
signals:
    void disconnected();
    void connected();
private:
    bool status;
    mqtt::async_client cli;
    Publisher publisher;
    Subscriber subscriber;
    Connection_Manager manager;
    static std::string SERVER_ADDRESS;
    static std::string CLIENT_ID;
    QThread mqttSubscribeThread;
    QThread mqttPublishThread;
    QThread mqttConnectionManagerThread;
};

#endif
