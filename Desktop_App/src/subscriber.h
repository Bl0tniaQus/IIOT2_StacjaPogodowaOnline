#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H
#include <QObject>
#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>
class Mqtt_Controller;
using json = nlohmann::json;
class Subscriber : public QObject
    {
    Q_OBJECT
    public:
        Subscriber();
        void setClient(mqtt::async_client* cli);
        void setController(Mqtt_Controller* ctrl);
        char getSubscribingStatus() {return subscribing;}
    public slots:
        void subscribe();
    signals:
        void sendMqttMessage(QString message);
    private:
        mqtt::async_client* client;
        QString checkJson(std::string jsonStr);
        char subscribing;
        Mqtt_Controller* controller;
    };
#endif
