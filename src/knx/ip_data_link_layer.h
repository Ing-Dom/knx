#pragma once

#include "config.h"
#ifdef USE_IP

#include <stdint.h>
#include "data_link_layer.h"
#include "ip_parameter_object.h"
#include "knx_ip_tunnel_connection.h"

class IpDataLinkLayer : public DataLinkLayer
{
    using DataLinkLayer::_deviceObject;

  public:
    IpDataLinkLayer(DeviceObject& devObj, IpParameterObject& ipParam, NetworkLayerEntity& netLayerEntity,
                    Platform& platform);

    void loop();
    void enabled(bool value);
    bool enabled() const;
    DptMedium mediumType() const override;
#ifdef KNX_TUNNELING
    void dataRequestToTunnel(CemiFrame& frame) override;
#endif

  private:
    bool _enabled = false;
    uint8_t _frameCount[10] = {0,0,0,0,0,0,0,0,0,0};
    uint8_t _frameCountBase = 0;
    uint32_t _frameCountTimeBase = 0;
    bool sendFrame(CemiFrame& frame);
#ifdef KNX_TUNNELING
    void sendFrameToTunnel(KnxIpTunnelConnection *tunnel, CemiFrame& frame);
#endif
    bool sendBytes(uint8_t* buffer, uint16_t length);
    bool isSendLimitReached();

    IpParameterObject& _ipParameters;

#ifdef KNX_TUNNELING
    KnxIpTunnelConnection tunnels[KNX_TUNNELING];
    uint8_t _lastChannelId = 1;
#endif
};
#endif