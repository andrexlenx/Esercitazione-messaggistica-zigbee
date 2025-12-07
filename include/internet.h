#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

extern String wifi_ssid;
extern String wifi_pass;

extern String tcp_host;
extern int tcp_port;

class InternetClass {
    public:
    bool connecting = false;
    bool tcpConnecting = false;
    bool error = false;
    String errorMessage = "";
    void (*onMessage)(uint8_t*, size_t) = nullptr;
    WiFiClient client;
    TaskHandle_t netask;

    void init_bridge();
    void stop_bridge();
    size_t write(const uint8_t *buf, size_t size);
    
    WiFiClient* getClient() {
        return &client;
    }

    TaskHandle_t* getTaskHandle() {
        return &netask;
    }

    static InternetClass* get_instance(){
        if(_instance == nullptr){
            _instance = new InternetClass();
        }
        return _instance;
    };

    private:
    static InternetClass* _instance;
    
    void reset_state() {
        tcpConnecting = false;
        connecting = false;
        error = false;
        errorMessage = "";
    }

    size_t read(uint8_t *buf, size_t size) {
        if (client.connected()){
            if (client.available()) {
                size_t readed = client.read(buf, size);
                if (onMessage) onMessage(buf, readed);
                return readed;
            }
            return 0;
        }
        return -1;
    }

    static void loop(void* parameter) {
        InternetClass* self = static_cast<InternetClass*>(parameter);
        while (self->client.connected()) {
            if (self->client.available()) {
                uint8_t buffer[4096];
                self->read(buffer, sizeof(buffer));
            }
            delay(50);
        }
    }
};

namespace Internet {
    inline void init_bridge(){ InternetClass::get_instance()->init_bridge(); };
    inline void stop_bridge(){ InternetClass::get_instance()->stop_bridge(); };
    inline size_t write(const uint8_t *buf, size_t size){ return InternetClass::get_instance()->write(buf, size); };
}