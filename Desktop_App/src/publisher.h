#ifndef PUBLISHER_H
#define PUBLISHER_H
#include <QObject>
#include <mqtt/async_client.h>
class Publisher : public QObject
    {
    Q_OBJECT
    public:
        Publisher();
        public slots:
            void requestUpdate();
            void setClient(mqtt::async_client* cli);
        private:
            mqtt::async_client* client;
    };
#endif

