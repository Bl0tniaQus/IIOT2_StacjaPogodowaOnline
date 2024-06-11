#include "publisher.h"
#include "mqtt_controller.h"
Publisher::Publisher()
{
    requestAllMessage = mqtt::make_message("M5Stack/IIOT/AH/request/all", "",1, false);
}//test
void Publisher::setClient(mqtt::async_client* cli)
{
    client = cli;
}
void Publisher::setController(Mqtt_Controller* ctrl)
{
	controller = ctrl;
}
void Publisher::requestUpdate()
{
    if (!isConnected()) return;
    mqtt::message_ptr msg = mqtt::make_message(controller->getTopicPrefix()+"request/all", "",1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}
void Publisher::changeTLB(int val)
{
    if (!isConnected()) return;
    mqtt::message_ptr msg = mqtt::make_message(controller->getTopicPrefix()+"set/temperature/lb", std::to_string(val),1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}
void Publisher::changeTUB(int val)
{
    if (!isConnected()) return;
    mqtt::message_ptr msg = mqtt::make_message(controller->getTopicPrefix()+"set/temperature/ub", std::to_string(val),1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}
void Publisher::changePLB(int val)
{
    if (!isConnected()) return;
    mqtt::message_ptr msg = mqtt::make_message(controller->getTopicPrefix()+"set/pressure/lb", std::to_string(val),1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}
void Publisher::changePUB(int val)
{
    if (!isConnected()) return;
    mqtt::message_ptr msg = mqtt::make_message(controller->getTopicPrefix()+"set/pressure/ub", std::to_string(val),1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}
void Publisher::changeHLB(int val)
{
    if (!isConnected()) return;
    mqtt::message_ptr msg = mqtt::make_message(controller->getTopicPrefix()+"set/humidity/lb", std::to_string(val),1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}
void Publisher::changeHUB(int val)
{
    if (!isConnected()) return;
    mqtt::message_ptr msg = mqtt::make_message(controller->getTopicPrefix()+"set/humidity/ub", std::to_string(val),1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}
bool Publisher::isConnected()
{
    return client->is_connected();
}

