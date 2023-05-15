#include "arduino_platform.h"

#include "Arduino.h"
#define DEBUG_ETHERNET_GENERIC_PORT         Serial
#define _ETG_LOGLEVEL_                      3
#define USING_SPI2                          false
#define ETHERNET_USE_RPIPICO      true
#define USE_THIS_SS_PIN       PIN_SPI0_SS  
#define SS_PIN_DEFAULT        USE_THIS_SS_PIN
#define ETHERNET_LARGE_BUFFERS
#include <Ethernet_Generic.hpp>
#include <EthernetClient.h>             // https://github.com/khoih-prog/Ethernet_Generic
#include <EthernetServer.h>             // https://github.com/khoih-prog/Ethernet_Generic
#include <EthernetUdp.h> 



#ifdef ARDUINO_ARCH_RP2040

#ifndef USE_RP2040_EEPROM_EMULATION
#ifndef KNX_FLASH_OFFSET
#define KNX_FLASH_OFFSET 0x180000   // 1.5MiB
#pragma warning "KNX_FLASH_OFFSET not defined, using 0x180000"
#endif
#endif

#ifdef USE_RP2040_LARGE_EEPROM_EMULATION
#define USE_RP2040_EEPROM_EMULATION
#endif

#if MASK_VERSION == 0x091A
#include <SPI.h>
#endif

class RP2040ArduinoPlatform : public ArduinoPlatform
{
public:
    RP2040ArduinoPlatform();
    RP2040ArduinoPlatform( HardwareSerial* s);

    void setupUart();

    // unique serial number
    uint32_t uniqueSerialNumber() override; 

    void restart();

    #ifdef USE_RP2040_EEPROM_EMULATION
    uint8_t* getEepromBuffer(uint16_t size);
    void commitToEeprom();

    #ifdef USE_RP2040_LARGE_EEPROM_EMULATION
    uint8_t _rambuff[KNX_FLASH_SIZE];
    bool _rambuff_initialized = false;
    #endif
    #else

    // size of one EraseBlock in pages
    virtual size_t flashEraseBlockSize();
    // size of one flash page in bytes
    virtual size_t flashPageSize();
    // start of user flash aligned to start of an erase block
    virtual uint8_t* userFlashStart();
    // size of the user flash in EraseBlocks
    virtual size_t userFlashSizeEraseBlocks();
    //relativ to userFlashStart
    virtual void flashErase(uint16_t eraseBlockNum);
    //write a single page to flash (pageNumber relative to userFashStart
    virtual void flashWritePage(uint16_t pageNumber, uint8_t* data); 
    
    // writes _eraseblockBuffer to flash - overrides Plattform::writeBufferedEraseBlock() for performance optimization only
    void writeBufferedEraseBlock();
    #endif
#if MASK_VERSION == 0x091A
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

  private:

    EthernetUDP _udp;
    IPAddress mcastaddr;
    uint16_t _port;
#endif
};

#endif
