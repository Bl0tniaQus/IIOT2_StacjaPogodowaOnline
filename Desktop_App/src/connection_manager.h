#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H
#include <QObject>
#include <mqtt/async_client.h>
#include <nlohmann/json.hpp>
#include <QThread>
class Mqtt_Controller;
class Connection_Manager : public QObject
    {
    Q_OBJECT
    public:
        Connection_Manager();
        void setClient(mqtt::async_client* cli);
        void setController(Mqtt_Controller* ctrl);
    public slots:
        void checkConnection();
    signals:
        void connected();
        void disconnected();
        void subscribeSignal();
    private:
        mqtt::async_client* client;
        Mqtt_Controller* controller;
        char first_time=1;
    };
#endif
