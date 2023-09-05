#pragma once

#include <cstdint>
#include "config.h"

#ifdef USE_IP

//TODO vervollständigen
enum ConnectionType : uint8_t
{
  Tunneling = 4
};

class KnxIpCRD
{
  public:
    KnxIpCRD(uint8_t* data);
    virtual ~KnxIpCRD();
    ConnectionType type() const;
    void type(ConnectionType value);
    void address(uint16_t addr);
    uint16_t address() const;
    uint8_t length() const;
    void length(uint8_t value);

  protected:
    uint8_t* _data = 0;
};
#endif
