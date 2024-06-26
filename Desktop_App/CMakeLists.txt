cmake_minimum_required(VERSION 3.0)
project(Desktop_App)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(PahoMqttCpp REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Qt5Charts)
set(desktop_app_SRC
  src/main.cpp
  src/desktop_app.cpp
  src/mqtt_controller.cpp
  src/subscriber.cpp
  src/publisher.cpp
  src/connection_manager.cpp
)

add_executable(desktop_app ${desktop_app_SRC})

target_link_libraries(desktop_app Qt5::Widgets Qt5::Charts libpaho-mqttpp3.a libpaho-mqtt3as.a nlohmann_json::nlohmann_json OpenSSL::SSL OpenSSL::Crypto)

# Install the executable
install(TARGETS desktop_app DESTINATION bin)
