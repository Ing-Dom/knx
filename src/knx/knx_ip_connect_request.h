#pragma once

#include "knx_ip_frame.h"
#include "ip_host_protocol_address_information.h"
#ifdef USE_IP
class KnxIpConnectRequest : public KnxIpFrame
{
  public:
    KnxIpConnectRequest(uint8_t* data, uint16_t length);
    IpHostProtocolAddressInformation& hpaiCtrl();
    IpHostProtocolAddressInformation& hpaiData();
  private:
    IpHostProtocolAddressInformation _hpaiCtrl;
    IpHostProtocolAddressInformation _hpaiData;
};
#endif