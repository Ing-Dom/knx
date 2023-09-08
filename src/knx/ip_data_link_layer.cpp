#include "config.h"
#ifdef USE_IP

#include "ip_data_link_layer.h"

#include "bits.h"
#include "platform.h"
#include "device_object.h"
#include "knx_ip_routing_indication.h"
#include "knx_ip_search_request.h"
#include "knx_ip_search_response.h"
#ifdef KNX_TUNNELING
#include "knx_ip_connect_request.h"
#include "knx_ip_connect_response.h"
#include "knx_ip_state_request.h"
#include "knx_ip_state_response.h"
#include "knx_ip_disconnect_request.h"
#include "knx_ip_disconnect_response.h"
#include "knx_ip_tunneling_request.h"
#include "knx_ip_tunneling_ack.h"
#endif

#include <stdio.h>
#include <string.h>

#define KNXIP_HEADER_LEN 0x6
#define KNXIP_PROTOCOL_VERSION 0x10

#define MIN_LEN_CEMI 10

IpDataLinkLayer::IpDataLinkLayer(DeviceObject& devObj, IpParameterObject& ipParam,
    NetworkLayerEntity &netLayerEntity, Platform& platform) : DataLinkLayer(devObj, netLayerEntity, platform), _ipParameters(ipParam)
{
}

bool IpDataLinkLayer::sendFrame(CemiFrame& frame)
{
    KnxIpRoutingIndication packet(frame);
    // only send 50 packet per second: see KNX 3.2.6 p.6
    if(isSendLimitReached())
        return false;
    bool success = sendBytes(packet.data(), packet.totalLength());
    dataConReceived(frame, success);
    return success;
}

void IpDataLinkLayer::loop()
{
    if (!_enabled)
        return;

    uint8_t buffer[512];
    int len = _platform.readBytesMultiCast(buffer, 512);
    if (len <= 0)
        return;

    if (len < KNXIP_HEADER_LEN)
        return;
    
    if (buffer[0] != KNXIP_HEADER_LEN 
        || buffer[1] != KNXIP_PROTOCOL_VERSION)
        return;

    uint16_t code;
    popWord(code, buffer + 2);
    switch ((KnxIpServiceType)code)
    {
        case RoutingIndication:
        {
            KnxIpRoutingIndication routingIndication(buffer, len);
            frameReceived(routingIndication.frame());
            break;
        }
        case SearchRequest:
        {
            println("Got SearchResponse");
            KnxIpSearchRequest searchRequest(buffer, len);
            KnxIpSearchResponse searchResponse(_ipParameters, _deviceObject);

            auto hpai = searchRequest.hpai();
            _platform.sendBytesUniCast(hpai.ipAddress(), hpai.ipPortNumber(), searchResponse.data(), searchResponse.totalLength());
            break;
        }
        case SearchRequestExt:
        {
            // FIXME, implement (not needed atm)
            break;
        }
#ifdef KNX_TUNNELING
        case ConnectRequest:
        {
            KnxIpConnectRequest connRequest(buffer, len);
    #ifdef KNX_LOG_IP
            println("Got Connect Request!");
            switch(connRequest.cri().type())
            {
                case DEVICE_MGMT_CONNECTION:
                    println("Device Management Connection");
                    break;
                case TUNNEL_CONNECTION:
                    println("Tunnel Connection");
                    break;
                case REMLOG_CONNECTION:
                    println("RemLog Connection");
                    break;
                case REMCONF_CONNECTION:
                    println("RemConf Connection");
                    break;
                case OBJSVR_CONNECTION:
                    println("ObjectServer Connection");
                    break;
            }
    #endif


            if(connRequest.cri().type() != TUNNEL_CONNECTION)
            {
                //We only support 0x04!
    #ifdef KNX_LOG_IP
            println("Only Tunnel Connection ist supported!");
    #endif
                KnxIpConnectResponse connRes(0x00, E_CONNECTION_TYPE);
                _platform.sendBytesUniCast(connRequest.hpaiCtrl().ipAddress(), connRequest.hpaiCtrl().ipPortNumber(), connRes.data(), connRes.totalLength());
                return;
            }

            if(connRequest.cri().layer() != 0x02) //LinkLayer
            {
                //We only support 0x02!
    #ifdef KNX_LOG_IP
            println("Only LinkLayer ist supported!");
    #endif
                KnxIpConnectResponse connRes(0x00, E_TUNNELING_LAYER);
                _platform.sendBytesUniCast(connRequest.hpaiCtrl().ipAddress(), connRequest.hpaiCtrl().ipPortNumber(), connRes.data(), connRes.totalLength());
                return;
            }

    #ifdef KNX_LOG_IP
            print("Data Endpoint: ");
            uint32_t ip = connRequest.hpaiData().ipAddress();
            print(ip >> 24);
            print(".");
            print((ip >> 16) & 0xFF);
            print(".");
            print((ip >> 8) & 0xFF);
            print(".");
            print(ip & 0xFF);
            print(":");
            println(connRequest.hpaiData().ipPortNumber());
            print("Ctrl Endpoint: ");
            ip = connRequest.hpaiCtrl().ipAddress();
            print(ip >> 24);
            print(".");
            print((ip >> 16) & 0xFF);
            print(".");
            print((ip >> 8) & 0xFF);
            print(".");
            print(ip & 0xFF);
            print(":");
            println(connRequest.hpaiCtrl().ipPortNumber());
    #endif

            KnxIpTunnelConnection *tun = nullptr;
            for(int i = 0; i < KNX_TUNNELING; i++)
            {
                if(tunnels[i].ChannelId == 0)
                {
                    tun = &tunnels[i];
                    break;
                }
            }

            if(tun == nullptr)
            {
    #ifdef KNX_LOG_IP
                println("Kein freier Tunnel verfügbar");
    #endif
                KnxIpConnectResponse connRes(0x00, E_NO_MORE_CONNECTIONS);
                _platform.sendBytesUniCast(connRequest.hpaiCtrl().ipAddress(), connRequest.hpaiCtrl().ipPortNumber(), connRes.data(), connRes.totalLength());
                return;
            }

            tun->ChannelId = _lastChannelId++;
            if(_lastChannelId == 0)
                _lastChannelId++;

    #ifdef KNX_LOG_IP
            print("Neuer Tunnel: 0x");
            print(tun->ChannelId, 16);
            print("/");
            print(tun->IndividualAddress >> 12);
            print(".");
            print((tun->IndividualAddress >> 8) & 0xF);
            print(".");
            println(tun->IndividualAddress & 0xFF);
    #endif

            tun->IpAddress = connRequest.hpaiData().ipAddress();
            tun->PortData = connRequest.hpaiData().ipPortNumber();
            tun->PortCtrl = connRequest.hpaiCtrl().ipPortNumber();

            KnxIpConnectResponse connRes(_ipParameters, tun->IndividualAddress, 3671, tun->ChannelId, connRequest.cri().type());

            _platform.sendBytesUniCast(tun->IpAddress, tun->PortCtrl, connRes.data(), connRes.totalLength());
            break;
        }

        case ConnectionStateRequest:
        {
            KnxIpStateRequest stateRequest(buffer, len);

            
            KnxIpTunnelConnection *tun = nullptr;
            for(int i = 0; i < KNX_TUNNELING; i++)
            {
                if(tunnels[i].ChannelId == stateRequest.channelId())
                {
                    tun = &tunnels[i];
                    break;
                }
            }

            if(tun == nullptr)
            {
    #ifdef KNX_LOG_IP
                println("Channel ID nicht gefunden");
    #endif
                KnxIpStateResponse stateRes(0x00, E_CONNECTION_ID);
                _platform.sendBytesUniCast(stateRequest.hpaiCtrl().ipAddress(), stateRequest.hpaiCtrl().ipPortNumber(), stateRes.data(), stateRes.totalLength());
                return;
            }

            //TODO check knx connection!
            //if no connection return E_KNX_CONNECTION

            //TODO check when to send E_DATA_CONNECTION

            tun->lastHeartbeat = millis();
            KnxIpStateResponse stateRes(tun->ChannelId, E_NO_ERROR);
            _platform.sendBytesUniCast(stateRequest.hpaiCtrl().ipAddress(), stateRequest.hpaiCtrl().ipPortNumber(), stateRes.data(), stateRes.totalLength());
            break;
        }

        case DisconnectRequest:
        {
            KnxIpDisconnectRequest discReq(buffer, len);
            
            KnxIpTunnelConnection *tun = nullptr;
            for(int i = 0; i < KNX_TUNNELING; i++)
            {
                if(tunnels[i].ChannelId == discReq.channelId())
                {
                    tun = &tunnels[i];
                    break;
                }
            }

            if(tun == nullptr)
            {
    #ifdef KNX_LOG_IP
                println("Channel ID nicht gefunden");
    #endif
                KnxIpDisconnectResponse discRes(0x00, E_CONNECTION_ID);
                _platform.sendBytesUniCast(discReq.hpaiCtrl().ipAddress(), discReq.hpaiCtrl().ipPortNumber(), discRes.data(), discRes.totalLength());
                return;
            }


            KnxIpDisconnectResponse discRes(tun->ChannelId, 0x00);
            _platform.sendBytesUniCast(discReq.hpaiCtrl().ipAddress(), discReq.hpaiCtrl().ipPortNumber(), discRes.data(), discRes.totalLength());
            tun->Reset();
            break;;
        }

        case TunnelingRequest:
        {
            KnxIpTunnelingRequest tunnReq(buffer, len);

            KnxIpTunnelConnection *tun = nullptr;
            for(int i = 0; i < KNX_TUNNELING; i++)
            {
                if(tunnels[i].ChannelId == tunnReq.connectionHeader().channelId())
                {
                    tun = &tunnels[i];
                    break;
                }
            }

            if(tun == nullptr)
            {
    #ifdef KNX_LOG_IP
                println("Channel ID nicht gefunden");
    #endif
                KnxIpStateResponse stateRes(0x00, E_CONNECTION_ID);
                //TODO where to send?
                //_platform.sendBytesUniCast(stateRequest.hpaiCtrl().ipAddress(), stateRequest.hpaiCtrl().ipPortNumber(), stateRes.data(), stateRes.totalLength());
                return;
            }

            if(tunnReq.connectionHeader().sequenceCounter() - 1 != tun->SequenceCounter)
            {
    #ifdef KNX_LOG_IP
                println("Wrong SequenceCounter: got %i expected %i", tunnReq.connectionHeader().sequenceCounter(), tun->SequenceCounter + 1);
    #endif
                //TODO überhaupt etwas zurück schicken?
                KnxIpTunnelingAck tunnAck;
                tunnAck.connectionHeader().channelId(tun->ChannelId);
                tunnAck.connectionHeader().sequenceCounter(tunnReq.connectionHeader().sequenceCounter());
                tunnAck.connectionHeader().status(E_ERROR);
                _platform.sendBytesUniCast(tun->IpAddress, tun->PortData, tunnAck.data(), tunnAck.totalLength());
                return;
            }

            tun->SequenceCounter = tunnReq.connectionHeader().sequenceCounter();

            if(tunnReq.frame().sourceAddress() == 0)
                tunnReq.frame().sourceAddress(tun->IndividualAddress);

            frameReceived(tunnReq.frame());

            KnxIpTunnelingAck tunnAck;
            tunnAck.connectionHeader().channelId(tun->ChannelId);
            tunnAck.connectionHeader().sequenceCounter(tunnReq.connectionHeader().sequenceCounter());
            tunnAck.connectionHeader().status(E_NO_ERROR);
            _platform.sendBytesUniCast(tun->IpAddress, tun->PortData, tunnAck.data(), tunnAck.totalLength());
    #ifdef KNX_LOG_IP
                println("Frame gesendet");
    #endif
            break;
        }
#endif
        default:
#ifdef KNX_LOG_IP
            print("Unhandled service identifier: ");
            println(code, HEX);
#endif
            break;
    }
}

void IpDataLinkLayer::enabled(bool value)
{
//    _print("own address: ");
//    _println(_deviceObject.individualAddress());
    if (value && !_enabled)
    {
        _platform.setupMultiCast(_ipParameters.propertyValue<uint32_t>(PID_ROUTING_MULTICAST_ADDRESS), KNXIP_MULTICAST_PORT);
#ifdef KNX_TUNNELING
//TODO USE PID 53
        uint16_t addr = _ipParameters.propertyValue<uint16_t>(PID_KNX_INDIVIDUAL_ADDRESS);
        uint8_t *addr = _ipParameters.propertyValue<uint8_t*>(PID_ADDITIONAL_INDIVIDUAL_ADDRESSES);
        for(int i = 0; i < KNX_TUNNELING; i++)
        {
            if((addr & 0xFF) == 255)
            {
                println("Es sind keine Adressen mehr frei für einen Tunnel!");
                break;
            }
            tunnels[i].IndividualAddress = ++addr;
    #ifdef KNX_LOG_IP
            print("Tunneling address: ");
            print(tunnels[i]IndividualAddress >> 12);
            print(".");
            print((tunnels[i]IndividualAddress >> 8) & 0xF);
            print(".");
            println(tunnels[i]IndividualAddress & 0xFF);
    #endif

        }
#endif
        _enabled = true;
        return;
    }

    if(!value && _enabled)
    {
        _platform.closeMultiCast();
        _enabled = false;
        return;
    }
}

bool IpDataLinkLayer::enabled() const
{
    return _enabled;
}

DptMedium IpDataLinkLayer::mediumType() const
{
    return DptMedium::KNX_IP;
}

bool IpDataLinkLayer::sendBytes(uint8_t* bytes, uint16_t length)
{
    if (!_enabled)
        return false;

    return _platform.sendBytesMultiCast(bytes, length);
}

bool IpDataLinkLayer::isSendLimitReached()
{
    uint32_t curTime = millis() / 100;

    // check if the countbuffer must be adjusted
    if(_frameCountTimeBase >= curTime)
    {
        uint32_t timeBaseDiff = _frameCountTimeBase - curTime;
        if(timeBaseDiff > 10)
            timeBaseDiff = 10;
        for(int i = 0; i < timeBaseDiff ; i++)
        {
            _frameCountBase++;
            _frameCountBase = _frameCountBase % 10;
            _frameCount[_frameCountBase] = 0;
        }
        _frameCountTimeBase = curTime;
    }
    else // _frameCountTimeBase < curTime => millis overflow, reset
    {
        for(int i = 0; i < 10 ; i++)
            _frameCount[i] = 0;
        _frameCountBase = 0;
        _frameCountTimeBase = curTime;
    }

    //check if we are over the limit
    uint16_t sum = 0;
    for(int i = 0; i < 10 ; i++)
        sum += _frameCount[i];
    if(sum > 50)
    {
        println("Dropping packet due to 50p/s limit");
        return true;   // drop packet
    }
    else
    {
        _frameCount[_frameCountBase]++;
        //print("sent packages in last 1000ms: ");
        //print(sum);
        //print(" curTime: ");
        //println(curTime);
        return false;
    }
}
#endif
