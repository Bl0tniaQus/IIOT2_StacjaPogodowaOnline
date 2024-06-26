#include "subscriber.h"
#include "mqtt_controller.h"
Subscriber::Subscriber()
{

}
void Subscriber::setClient(mqtt::async_client* cli)
{
    client = cli;
}
void Subscriber::setController(Mqtt_Controller* ctrl)
{
	controller = ctrl;
}

void Subscriber::subscribe()
{
	try {
		client->start_consuming();
		std::string prefix = controller->getTopicPrefix();
		std::cout<<prefix;
		client->subscribe(prefix+"all", 1)->wait();
		mqtt::message_ptr msg = mqtt::make_message(prefix+"request/all", "",1, false);
		client->publish(msg);
		while (true) {
			subscribing = 1;
			mqtt::const_message_ptr msg = client->consume_message();
			if (!msg) {break;}
			emit sendMqttMessage(checkJson(msg->to_string()));
		}
	}
	catch (const mqtt::exception& exc) {
        std::cerr << "\n  " << exc << std::endl;
	}
subscribing = 0;
}

QString Subscriber::checkJson(std::string jsonStr)
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
