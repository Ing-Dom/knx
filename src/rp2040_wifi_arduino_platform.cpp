/*-----------------------------------------------------

W5500 Ethernet Plattform extension for RP2040ArduinoPlatform

by Ing-Dom <dom@ingdom.de> 2023
credits to https://github.com/ecarjat 



----------------------------------------------------*/

#include "rp2040_wifi_arduino_platform.h"

#ifdef ARDUINO_ARCH_RP2040
#include "knx/bits.h"


RP2040WifiArduinoPlatform::RP2040WifiArduinoPlatform()
#ifndef KNX_NO_DEFAULT_UART
    : RP2040ArduinoPlatform(&KNX_SERIAL)
#endif
{

}

RP2040WifiArduinoPlatform::RP2040WifiArduinoPlatform( HardwareSerial* s) : RP2040ArduinoPlatform(s)
{

}

uint32_t RP2040WifiArduinoPlatform::currentIpAddress()
{
    return WiFi.localIP();
}
uint32_t RP2040WifiArduinoPlatform::currentSubnetMask()
{
    return WiFi.subnetMask();
}
uint32_t RP2040WifiArduinoPlatform::currentDefaultGateway()
{
    return WiFi.gatewayIP();
}
void RP2040WifiArduinoPlatform::macAddress(uint8_t* addr)
{
    WiFi.macAddress(addr);
}

// multicast
void RP2040WifiArduinoPlatform::setupMultiCast(uint32_t addr, uint16_t port)
{
    
    mcastaddr = IPAddress(htonl(addr));
    _port = port;
    uint8_t result = _udp.beginMulticast(mcastaddr, port);
    KNX_DEBUG_SERIAL.printf("Setup Mcast addr: ");
    mcastaddr.printTo(KNX_DEBUG_SERIAL);
    KNX_DEBUG_SERIAL.printf(" on port: %d result %d\n", port, result);
}

void RP2040WifiArduinoPlatform::closeMultiCast()
{
    _udp.stop();
}

bool RP2040WifiArduinoPlatform::sendBytesMultiCast(uint8_t* buffer, uint16_t len)
{
    //printHex("<- ",buffer, len);
    _udp.beginPacket(mcastaddr, _port);
    _udp.write(buffer, len);
    _udp.endPacket();
    return true;
}
int RP2040WifiArduinoPlatform::readBytesMultiCast(uint8_t* buffer, uint16_t maxLen)
{
    int len = _udp.parsePacket();
    if (len == 0)
        return 0;

    if (len > maxLen)
    {
        KNX_DEBUG_SERIAL.printf("udp buffer to small. was %d, needed %d\n", maxLen, len);
        fatalError();
    }

    _udp.read(buffer, len);
    
    KNX_DEBUG_SERIAL.printf("Remote IP: ");
    _udp.remoteIP().printTo(KNX_DEBUG_SERIAL);

    printHex("-> ", buffer, len);
    return len;
}

// unicast
bool RP2040WifiArduinoPlatform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len)
{
    
    IPAddress ucastaddr(htonl(addr));
    print("sendBytesUniCast to:");
    println(ucastaddr.toString().c_str());

    _udp_uni.begin(3671);
    if (_udp_uni.beginPacket(ucastaddr, port) == 1)
    {
        _udp_uni.write(buffer, len);
        if (_udp_uni.endPacket() == 0)
            println("sendBytesUniCast endPacket fail");
    }
    else
        println("sendBytesUniCast beginPacket fail");
    _udp_uni.stop();
    
    return true;
}

#endif


