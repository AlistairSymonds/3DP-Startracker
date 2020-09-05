#include <ESP8266WebServer.h>

class web_ui
{
private:
    ESP8266WebServer * server;
public:
    web_ui(ESP8266WebServer * server);
    ~web_ui();
};

