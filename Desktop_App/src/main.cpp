#include "desktop_app.h"
#include <QApplication>
#include <mqtt/async_client.h>
#include <string>
#include <pthread.h>
#include <iostream>
std::string SERVER_ADDRESS = "broker.mqttdashboard.com";
std::string CLIENT_ID =  "pirtesterxd";
std::string TOPIC = "pir/test/xd";
std::string text;
//todo qthread, połączenie mqtt + gui
const int  QOS = 1;
void* guiThread(void* arg);
void* mqttThread(void* arg);
int main(int argc, char *argv[])
{
    pthread_t guiT;
    pthread_t mqttT;
    int res = pthread_create(&mqttT,NULL,mqttThread, NULL);
    if (res != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
    QApplication app(argc, argv);
    Desktop_App w;
    w.show();
    return app.exec();
}
void* mqttThread(void* arg)
{
    mqtt::async_client cli(SERVER_ADDRESS, CLIENT_ID);

	auto connOpts = mqtt::connect_options_builder()
		.clean_session(false)
		.finalize();

	try {
		// Start consumer before connecting to make sure to not miss messages

		cli.start_consuming();

		// Connect to the server

		std::cout << "Connecting to the MQTT server..." << std::flush;
		auto tok = cli.connect(connOpts);

		// Getting the connect response will block waiting for the
		// connection to complete.
		auto rsp = tok->get_connect_response();

		// If there is no session present, then we need to subscribe, but if
		// there is a session, then the server remembers us and our
		// subscriptions.
		if (!rsp.is_session_present())
			cli.subscribe(TOPIC, QOS)->wait();

		std::cout << "OK" << std::endl;

		// Consume messages
		// This just exits if the client is disconnected.
		// (See some other examples for auto or manual reconnect)

		std::cout << "Waiting for messages on topic: '" << TOPIC << "'" << std::endl;

		while (true) {
			auto msg = cli.consume_message();
			if (!msg) break;
            std::cout << msg->get_topic() << ": " << msg->to_string() << std::endl;
		}

		// If we're here, the client was almost certainly disconnected.
		// But we check, just to make sure.

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

