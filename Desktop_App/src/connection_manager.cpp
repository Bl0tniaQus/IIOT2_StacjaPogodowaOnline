#include "connection_manager.h"
#include "mqtt_controller.h"
Connection_Manager::Connection_Manager()
{

}
void Connection_Manager::setClient(mqtt::async_client* cli)
{
    client = cli;
}
void Connection_Manager::setController(Mqtt_Controller* ctrl)
{
    controller = ctrl;
}
void Connection_Manager::checkConnection()
{

	while (true)
	{
		QThread::msleep(200);
		if (client->is_connected()) {

			if (!controller->getSubscriber()->getSubscribingStatus()) {
				emit subscribeSignal();
			}

			emit connected();
		}
		else if (!client->is_connected())
		{
			emit disconnected();
			mqtt::connect_options connOpts = mqtt::connect_options_builder().clean_session(true).finalize();
			mqtt::token_ptr tok = client->connect(connOpts);
			//auto rsp = tok->get_connect_response();
		}
	}
}
