#include <Arduino.h>
#include <iot_board.h>
#include <WiFi.h>
#include "internet.h"
#include "menu.h"
#include "bluetooth.cpp"

typedef struct __attribute__((packed)) mymsg_t {
  uint8_t command;
  uint8_t time[8];
  char username[16];
  char receiver[16];
  char message[486];
} mymsg_t;

mymsg_t lastMessage = {};
InternetClass *internet = InternetClass::get_instance();
WiFiClient *wifiClient = internet->getClient();
BLEConnector* bleConnector = new BLEConnector();

void onBleReceived(BLECharacteristic *pCharacteristic) {
    uint8_t* value = pCharacteristic->getData();
    size_t length = pCharacteristic->getLength();
    
    if (length > 0) {
      Serial.println("Received from BLE:");
      memcpy(&lastMessage, value, min(length, (size_t)sizeof(mymsg_t)));
      if (wifiClient->connected()) {
          Internet::write(value, length);
      }
    }
}

void onInetReceived(uint8_t* buf, size_t size) {
    memcpy(&lastMessage, buf, min(size, (size_t)sizeof(mymsg_t)));
    Serial.println("Received from network: ");
    if (bleConnector->allset) {
        bleConnector->sendData(buf, size);
    }
}

void LEDsoff(MenuItem* self, void* args) {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
}

void allLEDs(MenuItem* self, void* args) {
  int state = (int)args;
  digitalWrite(LED_RED, state);
  digitalWrite(LED_GREEN, state);
  digitalWrite(LED_YELLOW, state);
}

void toggleLED(MenuItem* self, void* args) {
  uint8_t ledPin = *((uint8_t*)args);
  digitalWrite(ledPin, digitalRead(ledPin) ^ 1);
}

void connectToNetwork(MenuItem* self, void* args) {
  if (wifiClient->connected()) {
    self->name = "WiFi connected";
    Menu::render();
    return;
  }
  self->name = "Connecting...";
  Menu::render();
  internet->init_bridge();
  self->name = String("state: ") + (wifiClient->connected() == 1 ? "Connected" : "Error");
}

void activateBLEServer(MenuItem* self, void* args) {
  if (bleConnector->allset) {
    self->name = "BLE active";
    Menu::render();
    return;
  }
  self->name = "Starting BLE...";
  Menu::render();
  bleConnector->init();
  self->name = bleConnector->allset ? "BLE Active" : "BLE Error";
}

MenuItem* ssidItem;
MenuItem* ipItem;
MenuItem* ipstatus;
void config_ipmenu(MenuItem* ipMenu){
  ssidItem = new MenuItem("SSID: " + wifi_ssid, ipMenu);
  ipItem = new MenuItem("IP: " + (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "Disconnected"), ipMenu);
  ipstatus = new MenuItem("Status: " + String((WiFi.status() == WL_CONNECTED) ? "Connected" : "Disconnected"), ipMenu);
}

MenuItem* siteItem;
MenuItem* tcpstatus;
void config_tcpmenu(MenuItem* tcpMenu){
  siteItem = new MenuItem("Endpoint: " + tcp_host, tcpMenu);
  tcpstatus = new MenuItem("Status: " + String((wifiClient->connected()) ? "Connected" : "Disconnected"), tcpMenu);
}

MenuItem* blestatus;
void config_blemenu(MenuItem* bleMenu){
  blestatus = new MenuItem("Status: " + String((bleConnector->allset) ? "Active" : "Inactive"), bleMenu);
}

String formatMessageTime(uint8_t* timeBytes) {
  uint64_t timeMs = 0;
  for (int i = 0; i < 8; i++) {
    timeMs |= ((uint64_t)timeBytes[i]) << (i * 8);
  }
  time_t rawTime = (time_t)(timeMs / 1000);
  struct tm * timeinfo;
  timeinfo = localtime(&rawTime);
  char buffer[20];
  strftime(buffer, 20, "%d/%m %H:%M", timeinfo);
  return String(buffer);
}

MenuItem* msgprotocol;
MenuItem* msgsender;
MenuItem* msgreceiver;
MenuItem* msgtime;
MenuItem* msgcontent;
void config_trafficmenu(MenuItem* TrafficMenu){
  msgprotocol = new MenuItem("Protocol: " + String(lastMessage.command), TrafficMenu);
  msgsender = new MenuItem("Sen: " + String(lastMessage.username), TrafficMenu);
  msgreceiver = new MenuItem("Rec: " + String(lastMessage.receiver), TrafficMenu);
  msgtime = new MenuItem("T: " + formatMessageTime(lastMessage.time), TrafficMenu);
  msgcontent = new MenuItem(String(lastMessage.message), TrafficMenu);
}

void update_info(){
  ipItem->name = "IP: " + (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : "Disconnected");
  ipstatus->name = "State: " + String((WiFi.status() == WL_CONNECTED) ? "Connected" : "Disconnected");

  tcpstatus->name = "State: " + String((wifiClient->connected()) ? "Connected" : "Disconnected");

  blestatus->name = "State: " + String((bleConnector->allset) ? "Active" : "Inactive");

  msgprotocol->name = "Protocol: " + String(lastMessage.command);
  msgsender->name = "Sen: " + String(lastMessage.username);
  msgreceiver->name = "Rec: " + String(lastMessage.receiver);
  msgtime->name = "T: " + formatMessageTime(lastMessage.time);
  msgcontent->name = String(lastMessage.message);
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

  internet->onMessage = onInetReceived;
  bleConnector->writeChar->ondata = onBleReceived;
  //Internet::init_bridge();
  MenuItem* rootMenu = new MenuItem("MAIN MENU");
  Menu::init_menu(rootMenu);

  MenuItem* networkcnn = new MenuItem("Network", rootMenu);
  networkcnn->action = connectToNetwork;

  MenuItem* blecnn = new MenuItem("BLE", rootMenu);
  blecnn->action = activateBLEServer;

  MenuItem* ledsMenu = new MenuItem("Toggle LEDs", rootMenu);
  ledsMenu->action = LEDsoff;

  MenuItem* infoMenu = new MenuItem("Info", rootMenu);
  MenuItem* ipMenu = new MenuItem("WiFi info", infoMenu);
  MenuItem* tcpMenu = new MenuItem("internet info", infoMenu);
  MenuItem* bleMenu = new MenuItem("BLE info", infoMenu);
  MenuItem* TrafficMenu = new MenuItem("Traffic info", infoMenu);
  config_ipmenu(ipMenu);
  config_tcpmenu(tcpMenu);
  config_blemenu(bleMenu);
  config_trafficmenu(TrafficMenu);
  update_info();

  MenuItem* redLEDItem = new MenuItem("Toggle RED", ledsMenu);
  redLEDItem->setAction(toggleLED, (uint8_t*)&LED_RED);
  
  MenuItem* yellowLEDItem = new MenuItem("Toggle YELLOW", ledsMenu);
  yellowLEDItem->setAction(toggleLED, (uint8_t*)&LED_YELLOW);

  MenuItem* greenLEDItem = new MenuItem("Toggle GREEN", ledsMenu);
  greenLEDItem->setAction(toggleLED, (uint8_t*)&LED_GREEN);

  MenuItem* allLEDsOnItem = new MenuItem("Turn All LEDs On", ledsMenu);
  allLEDsOnItem->setAction(allLEDs, (void*)HIGH);

  MenuItem* allLEDsOffItem = new MenuItem("Turn All LEDs Off", ledsMenu);
  allLEDsOffItem->setAction(allLEDs, (void*)LOW);

  Menu::render();
}

int counter = 0;

void loop() {
  buttons->update();
  update_display();
  delay(10);
}