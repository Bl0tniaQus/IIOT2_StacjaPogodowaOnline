#ifndef PUBLISHER_H
#define PUBLISHER_H
#include <QObject>
#include <mqtt/async_client.h>
class Mqtt_Controller;
class Publisher : public QObject
    {
    Q_OBJECT
    public:
        Publisher();
        public slots:
            void requestUpdate();
            void setClient(mqtt::async_client* cli);
            void changeTLB(int val);
            void changeTUB(int val);
            void changePLB(int val);
            void changePUB(int val);
            void changeHLB(int val);
            void changeHUB(int val);
            bool isConnected();
            void setController(Mqtt_Controller* ctrl);
        private:
            mqtt::async_client* client;
            mqtt::message_ptr requestAllMessage;
            Mqtt_Controller* controller;
    };
#endif

