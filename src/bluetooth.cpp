#include <Arduino.h>
#include <iot_board.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <map>

// random generated UUIDs
#define SERVICE_UUID        "b1f43047-1aae-4f6c-9429-74e61110dc84"
#define WRITE_CHARACTERISTIC_UUID "1e1701ee-62fe-45cc-8e74-331b3ef29cf7"
#define READ_CHARACTERISTIC_UUID  "c8b3d07a-c082-4580-988f-5a271ff60601"
#define DEVICE_NAME "BLE_Zigbee_Gateway"

class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer *server) override {
    Serial.println("Peer connected");
  }

  void onDisconnect(BLEServer *server) override {
    Serial.println("Peer disconnected");
  }
};

class BLEInboundCallback: public BLECharacteristicCallbacks {
    public:
        void (*ondata)(BLECharacteristic *pCharacteristic) = nullptr;
        void (*onotify)(BLECharacteristic *pCharacteristic) = nullptr;
    private:
        void onWrite(BLECharacteristic *pCharacteristic) {
            if (ondata) {
                ondata(pCharacteristic);
            }
            pCharacteristic->notify();
        }

        void onNotify(BLECharacteristic *pCharacteristic) {
            Serial.println("Inbound Characteristic Notify (ready for next write)");
            if (onotify) {
                onotify(pCharacteristic);
            }
        }
};

class BLEOutboundCallback: public BLECharacteristicCallbacks {
    public:
        void (*onread)(BLECharacteristic *pCharacteristic) = nullptr;
        void (*onotify)(BLECharacteristic *pCharacteristic) = nullptr;
    
    private:
        void onRead(BLECharacteristic *pCharacteristic) {
            Serial.println("Outbound Characteristic Read");
            if (onread) {
                onread(pCharacteristic);
            }
        }

        void onNotify(BLECharacteristic *pCharacteristic) {
            Serial.println("Outbound Characteristic Notify");
            if (onotify) {
                onotify(pCharacteristic);
            }
        }
};

class BLEConnector {
    public:
        BLEServer *pServer;
        BLEService *pService;
        BLEAdvertising *pAdvertising;
        BLECharacteristic *writeCharacteristic;
        BLECharacteristic *readCharacteristic;
        bool allset = false;
        BLEInboundCallback *writeChar = new BLEInboundCallback();
        BLEOutboundCallback *readChar = new BLEOutboundCallback();

    BLEConnector() {
        init();
    }

    void init(){
        BLEDevice::init(DEVICE_NAME);
    }

    void start(){
        pServer = BLEDevice::createServer();
        pService = pServer->createService(SERVICE_UUID);
        pServer->setCallbacks(new ServerCallbacks());
        writeCharacteristic = pService->createCharacteristic(
                                                WRITE_CHARACTERISTIC_UUID,
                                                BLECharacteristic::PROPERTY_NOTIFY |
                                                BLECharacteristic::PROPERTY_WRITE);

        readCharacteristic = pService->createCharacteristic(
                                                READ_CHARACTERISTIC_UUID,
                                                BLECharacteristic::PROPERTY_NOTIFY |
                                                BLECharacteristic::PROPERTY_READ);
        
        writeCharacteristic->setValue(0);
        writeCharacteristic->setCallbacks(writeChar);
        readCharacteristic->setCallbacks(readChar);
        pService->start();
        pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->setScanResponse(true);
        pAdvertising->setMinPreferred(0x06);
        pAdvertising->setMinPreferred(0x12);
        BLEDevice::startAdvertising();
        allset = true;
    }

    void stop(){
        BLEDevice::stopAdvertising();
        pService->stop();
        allset = false;
    }

    void release(){
        stop();
        BLEDevice::deinit(true);
    }

    void sendData(const uint8_t* data, size_t length){
        readCharacteristic->setValue(data, length);
        readCharacteristic->notify();
    }
};