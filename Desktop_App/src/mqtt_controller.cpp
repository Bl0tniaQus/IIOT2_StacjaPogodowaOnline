#include "mqtt_controller.h"
std::string Mqtt_Controller::SERVER_ADDRESS = "broker.mqttdashboard.com";
std::string Mqtt_Controller::CLIENT_ID = "M5StackAH";
Mqtt_Controller::Mqtt_Controller():cli(SERVER_ADDRESS,CLIENT_ID)
{
		auto connOpts = mqtt::connect_options_builder().clean_session(true).finalize();
		auto tok = cli.connect(connOpts);
		//auto rsp = tok->get_connect_response();
		subscriber.setClient(&cli);
		publisher.setClient(&cli);
		manager.setClient(&cli);
		manager.setController(this);
		subscriber.moveToThread(&mqttSubscribeThread);
		publisher.moveToThread(&mqttPublishThread);
		manager.moveToThread(&mqttConnectionManagerThread);
		mqttSubscribeThread.start();
		mqttPublishThread.start();
		mqttConnectionManagerThread.start();
}
Mqtt_Controller::~Mqtt_Controller()
{

}




