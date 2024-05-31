#include "mqtt_controller.h"
std::string Mqtt_Controller::SERVER_ADDRESS = "broker.mqttdashboard.com";
std::string Mqtt_Controller::TOPIC = "M5Stack/IIOT/AH/+";
std::string Mqtt_Controller::CLIENT_ID = "M5StackAH";
Mqtt_Controller::Mqtt_Controller():cli(SERVER_ADDRESS,CLIENT_ID)
{
		auto connOpts = mqtt::connect_options_builder()
		.clean_session(true)
		.finalize();
		auto tok = cli.connect(connOpts);
		auto rsp = tok->get_connect_response();
		qDebug("contrcon");
		subscriber.setClient(&cli);
		publisher.setClient(&cli);
		subscriber.moveToThread(&mqttSubscribeThread);
		publisher.moveToThread(&mqttPublishThread);
		mqttSubscribeThread.start();
		mqttPublishThread.start();
}
Mqtt_Controller::~Mqtt_Controller()
{

}
bool Mqtt_Controller::isConnected()
{
	return status;
}



