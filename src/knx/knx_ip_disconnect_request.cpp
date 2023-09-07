#include "knx_ip_disconnect_request.h"
#ifdef USE_IP
KnxIpDisconnectRequest::KnxIpDisconnectRequest(uint8_t* data, uint16_t length)
    : KnxIpFrame(data, length), _hpaiCtrl(data + LEN_KNXIP_HEADER + 1 /*ChannelId*/ + 1 /*Reserved*/)
{
}

IpHostProtocolAddressInformation& KnxIpDisconnectRequest::hpaiCtrl()
{
    return _hpaiCtrl;
}
uint8_t KnxIpDisconnectRequest::channelId()
{
    return _data[LEN_KNXIP_HEADER + 2];
}
#endif