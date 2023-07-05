#pragma once

#include "rp2040_arduino_platform.h"

#include "Arduino.h"




#if defined(ARDUINO_ARCH_RP2040) && defined(KNX_WIFI)

#include <WiFi.h>
#include <WiFiUdp.h>

#ifdef FUCK_LDF
#include "ldf_s_crap.h"
#endif


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

#endif
