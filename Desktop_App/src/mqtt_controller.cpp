#include "mqtt_controller.h"
std::string Mqtt_Controller::SERVER_ADDRESS = "broker.mqttdashboard.com";
std::string Mqtt_Controller::CLIENT_ID = "M5StackAH";
std::string Mqtt_Controller::TOPIC_PREFIX = "M5Stack/IIOT/AH/";

Mqtt_Controller::Mqtt_Controller():cli(SERVER_ADDRESS,CLIENT_ID)
{
		mqtt::connect_options connOpts = mqtt::connect_options_builder().clean_session(true).finalize();
		mqtt::token_ptr tok = cli.connect(connOpts);
		subscriber.setClient(&cli);
		subscriber.setController(this);
		publisher.setClient(&cli);
		publisher.setController(this);
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




