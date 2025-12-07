#include <Arduino.h>
#include <iot_board.h>
#include <WiFi.h>
#include "internet.h"
#include "menu.h"
#include "bluetooth.cpp"
#include "zigbee.cpp"

typedef struct __attribute__((packed)) mymsg_t {
  uint8_t command;
  uint8_t time[8];
  char username[16];
  char receiver[16];
  char message[470];
} mymsg_t;

mymsg_t lastMessage = {};
InternetClass *internet = InternetClass::get_instance();
WiFiClient *wifiClient = internet->getClient();
BLEConnector* bleConnector = new BLEConnector();
ZigbeeConnector* zigbeeConnector = new ZigbeeConnector();

void onBleReceived(BLECharacteristic *pCharacteristic) {
    uint8_t* value = pCharacteristic->getData();
    size_t length = pCharacteristic->getLength();
    
    if (length > 0) {
      Serial.println("Received from BLE:");
      memcpy(&lastMessage, value, min(length, (size_t)sizeof(mymsg_t)));
      if (wifiClient->connected()) {
          Internet::write(value, length);
      }
      if (zigbeeConnector->allset) {
          zigbeeConnector->sendBroadcast(value, length);
      }
    }
}

void onInetReceived(uint8_t* buf, size_t size) {
    memcpy(&lastMessage, buf, min(size, (size_t)sizeof(mymsg_t)));
    Serial.println("Received from network:");
    if (bleConnector->allset) {
        bleConnector->sendData(buf, size);
    }
    if (zigbeeConnector->allset) {
        zigbeeConnector->sendBroadcast(buf, size);
    }
}

void onZigbeeReceived(uint8_t* buf, size_t size) {
    memcpy(&lastMessage, buf, min(size, (size_t)sizeof(mymsg_t)));
    Serial.println("Received from Zigbee:");
    if (bleConnector->allset) {
        bleConnector->sendData(buf, size);
    }
    if (wifiClient->connected()) {
        Internet::write(buf, size);
    }
}

mymsg_t tmpmessage = {};
void testsend(MenuItem* self, void* args) {
  tmpmessage.command = (int)args;
  memcpy(tmpmessage.time, &lastMessage.time, 8);
  if (bleConnector->allset) {
      bleConnector->sendData((uint8_t*)&tmpmessage, sizeof(mymsg_t) - sizeof(tmpmessage.message) + strlen(tmpmessage.message) + 1);
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
  bleConnector->start();
  self->name = bleConnector->allset ? "BLE Active" : "BLE Error";
}

void activateZigbee(MenuItem* self, void* args) {
  if (zigbeeConnector->allset) {
    self->name = "Zigbee active";
    Menu::render();
    return;
  }
  self->name = "Starting Zigbee...";
  Menu::render();
  zigbeeConnector->init();
  self->name = zigbeeConnector->allset ? "Zigbee Active" : "Zigbee Error";
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

MenuItem* blemac;
MenuItem* blestatus;
void config_blemenu(MenuItem* bleMenu){
  blemac = new MenuItem("MAC: \n" + BLEDevice::getAddress().toString(), bleMenu);
  blestatus = new MenuItem("Status: " + String((bleConnector->allset) ? "Active" : "Inactive"), bleMenu);
}

MenuItem* zigbeeadr;
MenuItem* zigbeestatus;
void config_zigbeemenu(MenuItem* zigbeeMenu){
  zigbeeadr = new MenuItem("Address: " + String(zigbeeConnector->getAddress(), HEX), zigbeeMenu);
  zigbeestatus = new MenuItem("Status: " + String((zigbeeConnector->allset) ? "Active" : "Inactive"), zigbeeMenu);
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
  
  zigbeestatus->name = "State: " + String((zigbeeConnector->allset) ? "Active" : "Inactive");

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
  IoTBoard::init_spi();
  IoTBoard::init_zigbee();

  display->clearDisplay();
  Serial.println(String("BLE MAC Address: ") + BLEDevice::getAddress().toString());
  uint8_t* mac = BLEDevice::getAddress().getNative();
  zigbeeConnector->setAddress((uint16_t)(mac[4] << 8 | mac[5]));
  Serial.printf("Zigbee Address: %04x\n", zigbeeConnector->getAddress());

  tmpmessage.command = 128;
  u64_t timeoffset = 1765061214086;
  memcpy(tmpmessage.time, &timeoffset, 8);
  memset(&tmpmessage.username, 0, 16);
  memset(&tmpmessage.receiver, 0, 16);
  memset(&tmpmessage.message, 0, 470);
  strcpy(tmpmessage.username, "ESP32");
  strcpy(tmpmessage.receiver, "Global");
  strcpy(tmpmessage.message, "Hello from ESP32!");

  internet->onMessage = onInetReceived;
  bleConnector->writeChar->ondata = onBleReceived;

  MenuItem* rootMenu = new MenuItem("MAIN MENU");
  Menu::init_menu(rootMenu);

  MenuItem* networkcnn = new MenuItem("Network", rootMenu);
  networkcnn->action = connectToNetwork;

  MenuItem* blecnn = new MenuItem("BLE", rootMenu);
  blecnn->action = activateBLEServer;

  MenuItem* zigbeecnn = new MenuItem("Zigbee", rootMenu);
  zigbeecnn->action = activateZigbee;

  MenuItem* ledsMenu = new MenuItem("Toggle LEDs", rootMenu);
  ledsMenu->action = LEDsoff;

  MenuItem* infoMenu = new MenuItem("Info", rootMenu);
  MenuItem* ipMenu = new MenuItem("WiFi info", infoMenu);
  MenuItem* tcpMenu = new MenuItem("internet info", infoMenu);
  MenuItem* bleMenu = new MenuItem("BLE info", infoMenu);
  MenuItem* zigbeeMenu = new MenuItem("Zigbee info", infoMenu);
  MenuItem* TrafficMenu = new MenuItem("Traffic info", infoMenu);
  config_ipmenu(ipMenu);
  config_tcpmenu(tcpMenu);
  config_blemenu(bleMenu);
  config_zigbeemenu(zigbeeMenu);
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

  MenuItem* testmenu = new MenuItem("Send Test Msg", rootMenu);
  MenuItem* sendadvertisement = new MenuItem("Send Advertisement", testmenu);
  sendadvertisement->setAction(testsend, (void*)4);
  MenuItem* sendiscovery = new MenuItem("Send Discovery", testmenu);
  sendiscovery->setAction(testsend, (void*)8);
  MenuItem* sendrecv = new MenuItem("Send recv ack", testmenu);
  sendrecv->setAction(testsend, (void*)127);
  MenuItem* sendmessage = new MenuItem("Send Message", testmenu);
  sendmessage->setAction(testsend, (void*)128);
  MenuItem* sendbroadcast = new MenuItem("Send Broadcast", testmenu);
  sendbroadcast->setAction(testsend, (void*)255);

  Menu::render();
}

int counter = 0;

void loop() {
  buttons->update();
  update_display();
  delay(10);
}