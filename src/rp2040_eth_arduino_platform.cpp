/*-----------------------------------------------------

W5500 Ethernet Plattform extension for RP2040ArduinoPlatform

by Ing-Dom <dom@ingdom.de> 2023
credits to https://github.com/ecarjat 



----------------------------------------------------*/

#include "rp2040_eth_arduino_platform.h"

#ifdef ARDUINO_ARCH_RP2040
#include "knx/bits.h"


RP2040EthArduinoPlatform::RP2040EthArduinoPlatform()
#ifndef KNX_NO_DEFAULT_UART
    : RP2040ArduinoPlatform(&KNX_SERIAL)
#endif
{

}

RP2040EthArduinoPlatform::RP2040EthArduinoPlatform( HardwareSerial* s) : RP2040ArduinoPlatform(s)
{

}

uint32_t RP2040EthArduinoPlatform::currentIpAddress()
{
    return Ethernet.localIP();
}
uint32_t RP2040EthArduinoPlatform::currentSubnetMask()
{
    return Ethernet.subnetMask();
}
uint32_t RP2040EthArduinoPlatform::currentDefaultGateway()
{
    return Ethernet.gatewayIP();
}
void RP2040EthArduinoPlatform::macAddress(uint8_t* addr)
{
    Ethernet.MACAddress(addr);
}

// multicast
void RP2040EthArduinoPlatform::setupMultiCast(uint32_t addr, uint16_t port)
{
    
    mcastaddr = IPAddress(htonl(addr));
    _port = port;
    uint8_t result = _udp.beginMulticast(mcastaddr, port);

    print("Setup Mcast addr: ");
    print(mcastaddr.toString().c_str());
    print(" on port: ");
    print(port);
    print(" result ");
    println(result);
}

void RP2040EthArduinoPlatform::closeMultiCast()
{
    _udp.stop();
}

bool RP2040EthArduinoPlatform::sendBytesMultiCast(uint8_t* buffer, uint16_t len)
{
    //printHex("<- ",buffer, len);
    _udp.beginPacket(mcastaddr, _port);
    _udp.write(buffer, len);
    _udp.endPacket();
    return true;
}
int RP2040EthArduinoPlatform::readBytesMultiCast(uint8_t* buffer, uint16_t maxLen)
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
    
    print("Remote IP: ");
    print(_udp.remoteIP().toString().c_str());

    printHex("-> ", buffer, len);
    return len;
}

// unicast
bool RP2040EthArduinoPlatform::sendBytesUniCast(uint32_t addr, uint16_t port, uint8_t* buffer, uint16_t len)
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


