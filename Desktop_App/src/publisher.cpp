#include "publisher.h"
Publisher::Publisher()
{

}
void Publisher::setClient(mqtt::async_client* cli)
{
    client = cli;
}

void Publisher::requestUpdate()
{
    auto msg = mqtt::make_message("M5Stack/IIOT/AH/request/all", "1",1, false);
    client->publish(msg);
}
void Publisher::changeTLB(int val)
{
    auto msg = mqtt::make_message("M5Stack/IIOT/AH/set/temperature/lb", std::to_string(val),1, false);
    client->publish(msg);
}
void Publisher::changeTUB(int val)
{
    auto msg = mqtt::make_message("M5Stack/IIOT/AH/set/temperature/ub", std::to_string(val),1, false);
    client->publish(msg);
}
void Publisher::changePLB(int val)
{
    auto msg = mqtt::make_message("M5Stack/IIOT/AH/set/pressure/lb", std::to_string(val),1, false);
    client->publish(msg);
}
void Publisher::changePUB(int val)
{
    auto msg = mqtt::make_message("M5Stack/IIOT/AH/set/pressure/ub", std::to_string(val),1, false);
    client->publish(msg);
}
void Publisher::changeHLB(int val)
{
    auto msg = mqtt::make_message("M5Stack/IIOT/AH/set/humidity/lb", std::to_string(val),1, false);
    client->publish(msg);
}
void Publisher::changeHUB(int val)
{
    auto msg = mqtt::make_message("M5Stack/IIOT/AH/set/humidity/ub", std::to_string(val),1, false);
    client->publish(msg);
}

