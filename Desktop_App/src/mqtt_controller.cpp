#include "mqtt_controller.h"

Mqtt_Controller::Mqtt_Controller()
{
}
Mqtt_Controller::~Mqtt_Controller()
{

}


void Mqtt_Controller::mqttClient()
{
     mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

	auto connOpts = mqtt::connect_options_builder()
		.clean_session(false)
		.finalize();

	try {
		cli.start_consuming();
		//std::cout << "Connecting to the MQTT server..." << std::flush;
		auto tok = cli.connect(connOpts);
		auto rsp = tok->get_connect_response();
		if (!rsp.is_session_present())
			cli.subscribe(TOPIC, QOS)->wait();
		while (true) {
			auto msg = cli.consume_message();
			if (!msg) break;
           // std::cout << msg->get_topic() << ": " << msg->to_string() << std::endl;
			emit sendMqttMessage(QString::fromStdString(msg->to_string()));
		}
		if (cli.is_connected()) {
			std::cout << "\nShutting down and disconnecting from the MQTT server..." << std::flush;
			cli.unsubscribe(TOPIC)->wait();
			cli.stop_consuming();
			cli.disconnect()->wait();
			std::cout << "OK" << std::endl;
		}
		else {
			std::cout << "\nClient was disconnected" << std::endl;
		}
	}
	catch (const mqtt::exception& exc) {
        std::cerr << "\n  " << exc << std::endl;
	}

}
