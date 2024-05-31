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
	//mqtt::delivery_token_ptr tok = client->publish(msg);

    client->publish(msg);

}
