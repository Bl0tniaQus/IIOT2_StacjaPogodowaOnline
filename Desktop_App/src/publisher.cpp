#include "publisher.h"
Publisher::Publisher()
{
    requestAllMessage = mqtt::make_message("M5Stack/IIOT/AH/request/all", "1",1, false);
}
void Publisher::setClient(mqtt::async_client* cli)
{
    client = cli;
}

void Publisher::requestUpdate()
{
    mqtt::message_ptr msg = mqtt::make_message("M5Stack/IIOT/AH/request/all", "1",1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}
void Publisher::changeTLB(int val)
{
    mqtt::message_ptr msg = mqtt::make_message("M5Stack/IIOT/AH/set/temperature/lb", std::to_string(val),1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}
void Publisher::changeTUB(int val)
{
    mqtt::message_ptr msg = mqtt::make_message("M5Stack/IIOT/AH/set/temperature/ub", std::to_string(val),1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}
void Publisher::changePLB(int val)
{
    mqtt::message_ptr msg = mqtt::make_message("M5Stack/IIOT/AH/set/pressure/lb", std::to_string(val),1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}
void Publisher::changePUB(int val)
{
    mqtt::message_ptr msg = mqtt::make_message("M5Stack/IIOT/AH/set/pressure/ub", std::to_string(val),1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}
void Publisher::changeHLB(int val)
{
    mqtt::message_ptr msg = mqtt::make_message("M5Stack/IIOT/AH/set/humidity/lb", std::to_string(val),1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}
void Publisher::changeHUB(int val)
{
    mqtt::message_ptr msg = mqtt::make_message("M5Stack/IIOT/AH/set/humidity/ub", std::to_string(val),1, false);
    client->publish(msg);
    client->publish(requestAllMessage);
}

