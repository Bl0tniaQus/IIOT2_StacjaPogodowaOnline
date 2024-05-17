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
set(desktop_app_SRC
  src/main.cpp
  src/desktop_app.cpp
)

# Tell CMake to create the helloworld executable
add_executable(desktop_app ${desktop_app_SRC})

# Use the Widgets module from Qt 5.
target_link_libraries(desktop_app Qt5::Widgets libpaho-mqtt3as.a libpaho-mqttpp3.a OpenSSL::SSL OpenSSL::Crypto)

# Install the executable
install(TARGETS desktop_app DESTINATION bin)