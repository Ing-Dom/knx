#pragma once

#include "knx_ip_frame.h"
#include "knx_ip_crd.h"
#include "ip_host_protocol_address_information.h"
#include "knx_ip_device_information_dib.h"
#include "knx_ip_supported_service_dib.h"
#include "ip_parameter_object.h"
#ifdef USE_IP

enum KnxIpConnectionRequestErrorCodes
{
  E_NO_MORE_CONNECTIONS = 36
};

class KnxIpConnectResponse : public KnxIpFrame
{
  public:
    KnxIpConnectResponse(IpParameterObject& parameters, uint16_t address, uint16_t port, uint8_t channel);
    KnxIpConnectResponse(uint8_t channel, uint8_t errorCode);
    IpHostProtocolAddressInformation& controlEndpoint();
  private:
    IpHostProtocolAddressInformation _controlEndpoint;
    KnxIpCRD _crd;
};

#endif