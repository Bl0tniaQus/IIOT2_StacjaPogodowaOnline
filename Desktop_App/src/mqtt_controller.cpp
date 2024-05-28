#include "mqtt_controller.h"
Mqtt_Controller::Mqtt_Controller()
{

}
Mqtt_Controller::~Mqtt_Controller()
{

}
bool Mqtt_Controller::isConnected()
{
	return status;
}
void Mqtt_Controller::mqttClient()
{


	mqtt::async_client client(SERVER_ADDRESS,CLIENT_ID);
	auto connOpts = mqtt::connect_options_builder()
		.clean_session(false)
		.finalize();

	try {
		client.start_consuming();
		//std::cout << "Connecting to the MQTT server..." << std::flush;
		auto tok = client.connect(connOpts);
		auto rsp = tok->get_connect_response();
		if (!rsp.is_session_present())
		{client.subscribe(TOPIC, QOS)->wait();
			}
		if (client.is_connected()) {emit clientConnected();}
		while (true) {
			status = true;
			emit clientConnected();
			auto msg = client.consume_message();
			if (!msg) break;
			if (!client.is_connected()) {emit clientDisconnected();}
			emit sendMqttMessage(checkJson(msg->to_string()));
		}
		if (client.is_connected()) {
			std::cout << "\nShutting down and disconnecting from the MQTT server..." << std::flush;
			client.unsubscribe(TOPIC)->wait();
			client.stop_consuming();
			client.disconnect()->wait();
			std::cout << "OK" << std::endl;
			status=false;
			emit clientDisconnected();

		}
		else {
			status=false;
			emit clientDisconnected();
		}
	}
	catch (const mqtt::exception& exc) {
        std::cerr << "\n  " << exc << std::endl;
	}

}
QString Mqtt_Controller::checkJson(std::string jsonStr)
{

	try {
    json msg = json::parse(jsonStr);

	return QString::fromStdString(jsonStr);
	}
   catch (json::parse_error e)
   {
    return QString::fromStdString(
		R"({
      "temp_LB": "null",
      "temp": "null",
      "temp_UB": "null",
      "pres_LB": "null",
      "pres": "null",
      "pres_UB": "null",
	  "hum_LB": "null",
      "hum": "null",
      "hum_UB": "null"
    })"
	);
	}
}
