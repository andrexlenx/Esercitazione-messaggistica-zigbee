#include <Arduino.h>
#include <iot_board.h>
#include <WiFi.h>
#include "internet.h"

String wifi_ssid = SSID;
String wifi_pass = PASSWORD;

String tcp_host = HOST;
int tcp_port = PORT;

#define timeoutWiFiConnect 50000

InternetClass * InternetClass::_instance = nullptr;

void InternetClass::init_bridge() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
    Serial.print("[NET] Connecting to WiFi ");
    Serial.println(wifi_ssid);
    connecting = true;
    tcpConnecting = true;
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeoutWiFiConnect) {
        delay(500);
        Serial.print(WiFi.status());
        Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() != WL_CONNECTED) {
        errorMessage = "Failed to connect to "+ wifi_ssid + ", status code: " + String(WiFi.status());
        error = true;
        Serial.println(errorMessage);
        connecting = false;
        return;
    }
    connecting = false;
    Serial.print("[NET] Connecting to node ");
    Serial.println(tcp_host);
    int cnsuccess = client.connect(tcp_host.c_str(), tcp_port);
    if (!cnsuccess) {
        errorMessage = "Connection to host " + tcp_host + ":" + String(tcp_port) + " failed";
        error = true;
        Serial.println(errorMessage);
        tcpConnecting = false;
        return;
    }
    tcpConnecting = false;
    xTaskCreate(loop, "InternetTask", 5120, this, 1, &netask);
    Serial.println("[NET] Bridge OK");
}

void InternetClass::stop_bridge() {
    if (client.connected()) {
        client.stop();
    }
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
    }
    reset_state();
}

size_t InternetClass::write(const uint8_t *buf, size_t size) {
    if (client.connected()) {
        return client.write(buf, size);
    }
        return -1;
}