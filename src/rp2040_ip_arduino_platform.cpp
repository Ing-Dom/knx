/*-----------------------------------------------------

W5500 Ethernet Plattform extension for RP2040ArduinoPlatform

by Ing-Dom <dom@ingdom.de> 2023
credits to https://github.com/ecarjat 

needs arduino-pico - "Raspberry Pi Pico Arduino core, for all RP2040 boards"
by Earl E. Philhower III https://github.com/earlephilhower/arduino-pico 
>= V3.6.0



----------------------------------------------------*/




#include "rp2040_ip_arduino_platform.h"

#if ARDUINO_PICO_MAJOR * 10000 + ARDUINO_PICO_MINOR * 100 + ARDUINO_PICO_REVISION < 30600
#pragma error "arduino-pico >= 3.6.0 needed"
#endif

#if defined(ARDUINO_ARCH_RP2040)
#include "knx/bits.h"

#ifdef KNX_IP_W5500

extern Wiznet5500lwIP KNX_NETIF;

#elif defined(KNX_IP_WIFI)



#endif



RP2040IpArduinoPlatform::RP2040IpArduinoPlatform()
#ifndef KNX_NO_DEFAULT_UART
    : RP2040ArduinoPlatform(&KNX_SERIAL)
#endif
{

}

RP2040IpArduinoPlatform::RP2040IpArduinoPlatform( HardwareSerial* s) : RP2040ArduinoPlatform(s)
{

}

uint32_t RP2040IpArduinoPlatform::currentIpAddress()
{
    return KNX_NETIF.localIP();
}
uint32_t RP2040IpArduinoPlatform::currentSubnetMask()
{
    return KNX_NETIF.subnetMask();
}
uint32_t RP2040IpArduinoPlatform::currentDefaultGateway()
{
    return KNX_NETIF.gatewayIP();
}
void RP2040IpArduinoPlatform::macAddress(uint8_t* addr)
{
#if defined(KNX_IP_W5500)
    addr = KNX_NETIF.getNetIf()->hwaddr;
#elif defined(KNX_IP_WIFI)
    addr = KNX_NETIF.macAddress(_macaddr);
#endif
}

// multicast
void RP2040IpArduinoPlatform::setupMultiCast(uint32_t addr, uint16_t port)
{
    
    mcastaddr = IPAddress(htonl(addr));
    _port = port;
    uint8_t result = _udp.beginMulticast(mcastaddr, port);

#ifdef KNX_LOG_IP
    print("Setup Mcast addr: ");
    print(mcastaddr.toString().c_str());
    print(" on port: ");
    print(port);
    print(" result ");
    println(result);
#endif
}

void RP2040IpArduinoPlatform::closeMultiCast()
{
    _udp.stop();
}

bool RP2040IpArduinoPlatform::sendBytesMultiCast(uint8_t* buffer, uint16_t len)
{
#ifdef KNX_LOG_IP
    printHex("<- ",buffer, len);
#endif
    //ToDo: check if Ethernet is able to receive
    _udp.beginPacket(mcastaddr, _port);
    _udp.write(buffer, len);
    _udp.endPacket();
    return true;
}
int RP2040IpArduinoPlatform::readBytesMultiCast(uint8_t* buffer, uint16_t maxLen)
{
    int len = _udp.parsePacket();
    if (len == 0)
        return 0;

    if (len > maxLen)
    {
        print("udp buffer to small. was ");
        print(maxLen);
        print(", needed ");
        println(len);
        fatalError();
    }

    _udp.read(buffer, len);
#ifdef KNX_LOG_IP
    print("Remote IP: ");
    print(_udp.remoteIP().toString().c_str());

    printHex("-> ", buffer, len);
#endif
    return len;
}

// unicast
bool RP2040IpArduinoPlatform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len)
{
    IPAddress ucastaddr(htonl(addr));
    
#ifdef KNX_LOG_IP
    print("sendBytesUniCast to:");
    println(ucastaddr.toString().c_str());
#endif

    if (_udp.beginPacket(ucastaddr, port) == 1)
    {
        _udp.write(buffer, len);
        if (_udp.endPacket() == 0)
            println("sendBytesUniCast endPacket fail");
    }
    else
        println("sendBytesUniCast beginPacket fail");

    return true;
}

#endif


