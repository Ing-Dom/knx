#pragma once

#include "rp2040_arduino_platform.h"

#include "Arduino.h"



#if defined(ARDUINO_ARCH_RP2040)

#ifdef KNX_IP_W5500

#define KNX_NETIF Eth
#include "SPI.h"
#include <W5500lwIP.h>

#elif defined(KNX_IP_WIFI)

#define KNX_NETIF WiFi
#include <WiFi.h>
#include <WiFiUdp.h>

#endif


class RP2040IpArduinoPlatform : public RP2040ArduinoPlatform
{
public:

    RP2040IpArduinoPlatform();
    RP2040IpArduinoPlatform( HardwareSerial* s);


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
    IPAddress mcastaddr;
    uint16_t _port;
    uint8_t _macaddr[6];

};

#endif
