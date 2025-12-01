#include <Arduino.h>
#include <iot_board.h>
#include <WiFi.h>
#include "internet.h"
#include "menu.h" 

String lastMessage = "";
InternetClass *internet = InternetClass::get_instance();
WiFiClient *wifiClient = internet->getClient();

void onMessageReceived(uint8_t* buf, size_t size) {
    char buffer[size + 1];
    memcpy(buffer, buf, size);
    buffer[size] = '\0';
    String line = String(buffer);
    lastMessage = line;
    Serial.println("Received: " + line);
}

void LEDsoff(MenuItem* self, void* args) {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
}

void toggleLED(MenuItem* self, void* args) {
  uint8_t ledPin = *((uint8_t*)args);
  digitalWrite(ledPin, digitalRead(ledPin) ^ 1);
}

void connectToNetwork(MenuItem* self, void* args) {
  if (wifiClient->connected()) {
    self->name = "Already connected";
    Menu::render();
    return;
  }
  self->name = "Connecting...";
  Menu::render();
  internet->init_bridge();
  self->name = String("state: ") + (wifiClient->connected() == 1 ? "Connected" : "Error");
}

MenuItem* ipitem;
MenuItem* tcpstatus;
MenuItem* lastmsg;

void update_info(){
  ipitem->name = String(wifi_ssid) + (WiFi.status() == WL_CONNECTED ? " IP: " + WiFi.localIP().toString() : " Disconnected");
  tcpstatus->name = String("endpoint: ") + tcp_host + String(" status: ") + (wifiClient->connected() ? "Connected" : "Disconnected");
  lastmsg->name = "Last msg: " + lastMessage;
}

void update_display() {
  update_info();
  Menu::render();
}

void setup() {
  IoTBoard::init_serial();
  IoTBoard::init_leds();
  IoTBoard::init_display();
  IoTBoard::init_buttons();

  internet->onMessage = onMessageReceived;
  //Internet::init_bridge();
  MenuItem* rootMenu = new MenuItem("MAIN MENU");
  Menu::init_menu(rootMenu);

  MenuItem* networkMenu = new MenuItem("Network", rootMenu);
  networkMenu->action = connectToNetwork;

  MenuItem* ledsMenu = new MenuItem("Toggle LEDs", rootMenu);
  ledsMenu->action = LEDsoff;

  MenuItem* infoMenu = new MenuItem("Info", rootMenu);
  ipitem = new MenuItem("WiFi info", infoMenu);
  tcpstatus = new MenuItem("Network Node", infoMenu);
  lastmsg = new MenuItem("Last Msg", infoMenu);
  update_info();

  MenuItem* redLEDItem = new MenuItem("Toggle RED", ledsMenu);
  redLEDItem->setAction(toggleLED, (uint8_t*)&LED_RED);
  
  MenuItem* yellowLEDItem = new MenuItem("Toggle YELLOW", ledsMenu);
  yellowLEDItem->setAction(toggleLED, (uint8_t*)&LED_YELLOW);

  MenuItem* greenLEDItem = new MenuItem("Toggle GREEN", ledsMenu);
  greenLEDItem->setAction(toggleLED, (uint8_t*)&LED_GREEN);

  Menu::render();
}

int counter = 0;

void loop() {
  buttons->update();
  update_display();
  char buffer[128];
  if (wifiClient->connected()) {
    if (counter >= 300){
      snprintf(buffer, sizeof(buffer), "Hello from %s at %lu\n", WiFi.localIP().toString().c_str(), millis());
      Internet::write((uint8_t *)buffer, strlen(buffer));
      Serial.println("Sent: " + String(buffer));
    }
    counter = (counter + 1) % 301;
  }
  delay(10);
}