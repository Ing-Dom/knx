#pragma once

#include "rp2040_arduino_platform.h"

#include "Arduino.h"



#if defined(ARDUINO_ARCH_RP2040) && defined(KNX_ETH_GEN)


#include <SPI.h>

#ifndef DEBUG_ETHERNET_GENERIC_PORT
#define DEBUG_ETHERNET_GENERIC_PORT         Serial
#endif

#ifndef _ETG_LOGLEVEL_
#define _ETG_LOGLEVEL_                      1
#endif

// set to true if you want to use SPI1, otherwise SPI is used.
//#define ETHERNET_GENERIC_USING_SPI2                          false

#define ETHERNET_USE_RPIPICO      true
#define ETHERNET_LARGE_BUFFERS
#include <Ethernet_Generic.hpp>
#include <EthernetClient.h>             // https://github.com/khoih-prog/Ethernet_Generic
#include <EthernetServer.h>             // https://github.com/khoih-prog/Ethernet_Generic
#include <EthernetUdp.h>


class RP2040EthArduinoPlatform : public RP2040ArduinoPlatform
{
public:

    RP2040EthArduinoPlatform();
    RP2040EthArduinoPlatform( HardwareSerial* s);


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

    EthernetUDP _udp;
    EthernetUDP _udp_uni;
    bool _unicast_socket_setup = false;
    IPAddress mcastaddr;
    uint16_t _port;

};

#endif
