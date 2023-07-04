#pragma once

#include "rp2040_arduino_platform.h"

#include "Arduino.h"



#ifdef ARDUINO_ARCH_RP2040
//#if MASK_VERSION == 0x091A || MASK_VERSION == 0x57B0

#include <WiFi.h>
#include <WiFiUdp.h>


class RP2040WifiArduinoPlatform : public RP2040ArduinoPlatform
{
public:

    RP2040WifiArduinoPlatform();
    RP2040WifiArduinoPlatform( HardwareSerial* s);


    uint32_t currentIpAddress() override;
    uint32_t currentSubnetMask() override;
    uint32_t currentDefaultGateway() override;
    void macAddress(uint8_t* addr) override;

    // multicast
    void setupMultiCast(uint32_t addr, uint16_t port) override;
    void closeMultiCast() override;
    bool sendBytesMultiCast(uint8_t* buffer, uint16_t len) override;
    int readBytesMultiCast(uint8_t* buffer, uint16_t maxLen) override;

    // unicast
    bool sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len) override;

  protected:

    WiFiUDP _udp;
    WiFiUDP _udp_uni;
    IPAddress mcastaddr;
    uint16_t _port;

};
//#endif
#endif
