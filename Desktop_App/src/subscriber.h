#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H
#include <QObject>
#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
class Subscriber : public QObject
    {
    Q_OBJECT
    public:
        Subscriber();
        void setClient(mqtt::async_client* cli);
        char getSubscribingStatus() {return subscribing;}
    public slots:
        void subscribe();
    signals:
        void sendMqttMessage(QString message);
    private:
        mqtt::async_client* client;
        QString checkJson(std::string jsonStr);
        std::string topic = "M5Stack/IIOT/AH/+";
        char subscribing;
    };
#endif
