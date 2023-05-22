#include "rp2040_arduino_platform.h"

#include "Arduino.h"



#ifdef ARDUINO_ARCH_RP2040

#include <SPI.h>
#define DEBUG_ETHERNET_GENERIC_PORT         Serial
#define _ETG_LOGLEVEL_                      3
#define USING_SPI2                          true
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
    IPAddress mcastaddr;
    uint16_t _port;

};

#endif
