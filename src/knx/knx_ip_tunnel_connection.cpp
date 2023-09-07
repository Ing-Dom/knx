#include "knx_ip_tunnel_connection.h"

KnxIpTunnelConnection::KnxIpTunnelConnection()
{

}

void KnxIpTunnelConnection::Reset()
{
    ChannelId = 0;
    IndividualAddress = 0;
    IpAddress = 0;
    PortData = 0;
    PortCtrl = 0;
    lastHeartbeat = 0;
}
