#include <iot_board.h>
#include <atomic.h>
#include <vector>

#define ZIGBEE_ADDRESS 0xAE
#define ZIGBEE_PANID 0x804D
#define ZIGBEE_CHANNEL 12
#define SEND_TIMEOUT 2000

class ZigbeeConnector {
    
    public:
    bool allset = false;
    uint16_t addr;
    uint16_t panId;
    int channel;
    void (*ondata)(uint8_t* data, size_t size) = nullptr;
    TaskHandle_t zigtask;

    ZigbeeConnector(uint16_t addr = ZIGBEE_ADDRESS, uint16_t panId = ZIGBEE_PANID, int channel = ZIGBEE_CHANNEL) : addr(addr), panId(panId), channel(channel) {}

    void init(){
        zigbee->setAddress(addr);
        zigbee->setPanId(panId);
        zigbee->setChannel(channel);
        allset = true;
        xTaskCreate(Receiver, "ZigbeeTask", 4096, this, 1, &zigtask);
    }

    void setAddress(uint16_t addr){
        this->addr = addr;
        zigbee->setAddress(addr);
    }
    
    void setPanId(uint16_t panId){
        this->panId = panId;
        zigbee->setPanId(panId);
    }

    void setChannel(int channel){
        this->channel = channel;
        zigbee->setChannel(channel);
    }

    uint16_t getAddress(){
        return addr;
    }

    uint16_t getPanId(){
        return panId;
    }

    int getChannel(){
        return channel;
    }

    String getAddressString(){
        char buf[5];
        sprintf(buf, "%02X%02X", (addr >> 8) & 0xFF, addr & 0xFF);
        return String(buf);
    }

    void registerAddress(uint16_t addr){
        if (std::find(registered_addrs.begin(), registered_addrs.end(), addr) == registered_addrs.end()){
            registered_addrs.push_back(addr);
        }
    }

    void unregisterAddress(uint16_t addr){
        registered_addrs.erase(std::remove(registered_addrs.begin(), registered_addrs.end(), addr), registered_addrs.end());
    }

    void sendData(uint8_t* data, size_t size, uint16_t dest_addr){
        _setupsend(data, size);
        zigbee->sendPacket(dest_addr);
        waitForAck();
    }

    void sendAll(uint8_t* data, size_t size){
        for (auto addr : registered_addrs){
            _setupsend(data, size);
            zigbee->sendPacket(addr);
            waitForAck();
        }
    }

    void sendBroadcast(uint8_t* data, size_t size){
        _setupsend(data, size);
        zigbee->sendPacket(0xFFFF);
        waitForAck();
    }

    private:
    std::vector<uint16_t> registered_addrs;
    
    void _setupsend(uint8_t* data, size_t size){
        zigbee->startPacket();
        zigbee->writeBytes(data, size);
    }
    
    void waitForAck(){
        unsigned long startTime = millis();
        while(!zigbee->transmissionSuccess() && (millis() - startTime) < SEND_TIMEOUT){
            delay(10);
        }
    }
    static void Receiver(void *pvParameters){
        ZigbeeConnector* self = static_cast<ZigbeeConnector*>(pvParameters);
        while(self->allset){
            if(zigbee->receivePacket()){
                uint8_t received[1024];
                zigbee->readBytes(received, sizeof(received));
                if (self->ondata) self->ondata(received, sizeof(received));
            }
            delay(100);
        }
    }
};