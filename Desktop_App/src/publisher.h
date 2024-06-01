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
            void changeTLB(int val);
            void changeTUB(int val);
            void changePLB(int val);
            void changePUB(int val);
            void changeHLB(int val);
            void changeHUB(int val);

        private:
            mqtt::async_client* client;
    };
#endif

