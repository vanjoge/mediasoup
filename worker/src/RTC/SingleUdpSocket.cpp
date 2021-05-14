#define MS_CLASS "RTC::UdpSocket"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/SingleUdpSocket.hpp"
#include "Logger.hpp"
#include "RTC/PortManager.hpp"
#include "RTC/StunPacket.hpp"
#include "Utils.hpp"
#include <string>
/// <summary>
/// 
/// </summary>
namespace RTC
{
    /* Instance methods. */
    static constexpr size_t StunBufferSize{ 65536 };
    static uint8_t StunBuffer[StunBufferSize];


    SingleUdpSocket::SingleUdpSocket(std::string& ip)
    {
        MS_TRACE();
        this->udpSocket = new RTC::UdpSocket(this, ip, 1);
    }

    SingleUdpSocket::~SingleUdpSocket()
    {
        MS_TRACE();
        if (this->udpSocket)
        {
            delete udpSocket;
        }
    }

    void SingleUdpSocket::OnUdpSocketPacketReceived(
      RTC::UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr)
    {
        std::string peer_id;
        GetPeerId(remoteAddr, peer_id);

        RTC::UdpSocket::Listener  *listener = NULL;

        auto peer_iter = mapTransportPeerId.find(peer_id);
        if (peer_iter != mapTransportPeerId.end())
        {
            listener = peer_iter->second;
        }

        if (listener)
        {
            listener->OnUdpSocketPacketReceived(socket, data, len, remoteAddr);
            return;
        }

        RTC::StunPacket* packet = RTC::StunPacket::Parse(data, len);

        if (!packet)
        {
            MS_WARN_DEV("ignoring wrong STUN packet received");
            return;
        }
        std::string username = packet->GetUsername();
        auto size            = username.find(":");
        if (size != std::string::npos)
        {
            username = username.substr(0, size);
        }
        auto name_iter = mapTransportName.find(username);
        if (name_iter == mapTransportName.end())
        {
            // Reply 400.
            RTC::StunPacket* response = packet->CreateErrorResponse(400);
            response->Serialize(StunBuffer);
            socket->Send(response->GetData(),response->GetSize(),remoteAddr,NULL);
            delete packet;
            delete response;
            return;
        }

        listener = name_iter->second;
        SetTransportByPeerId(listener, peer_id);

        if (listener)
        {
            listener->OnUdpSocketPacketReceived(socket, data, len, remoteAddr);
        }
            delete packet;
    }

    void SingleUdpSocket::SetTransportByUserName(
      RTC::UdpSocket::Listener* listener, const std::string& name)
    {
        mapTransportName[name] = listener;
    }

    void SingleUdpSocket::SetTransportByPeerId(RTC::UdpSocket::Listener* listener, const std::string& peerId)
    {
        mapTransportPeerId[peerId] = listener;
    }

    void SingleUdpSocket::GetPeerId(const struct sockaddr* remoteAddr, std::string& peer_id)
    {
        int family = 0;
        std::string ip;
        uint16_t port;
        Utils::IP::GetAddressInfo(remoteAddr, family, ip, port);

        char id_buf[512] = { 0 };
        int len = snprintf(id_buf, sizeof(id_buf), "%s:%d", ip.c_str(), port);
        peer_id = std::move(std::string(id_buf, len));
    }


    void SingleUdpSocket::ChangeTransportByUserName(
      RTC::UdpSocket::Listener* listener, const std::string& old_name, const std::string& new_name)
    {
        auto name_iter = mapTransportName.find(old_name);
        if (name_iter != mapTransportName.end())
        {
            mapTransportName.erase(name_iter);
        }
        mapTransportName[new_name] = listener;
    }

    uint16_t SingleUdpSocket::GetLocalPort()
    {
        return udpSocket->GetLocalPort();
    }

    std::string SingleUdpSocket::GetLocalIp()
    {
        return udpSocket->GetLocalIp();
    }

    void SingleUdpSocket::DeleteTransport(RTC::UdpSocket::Listener* listener)
    {

        for (auto iter = mapTransportName.begin(); iter != mapTransportName.end();)
        {
            if (iter->second == listener)
            {
                mapTransportName.erase(iter++);
            }
            else
            {
                iter++;
            }
        }

        for (auto iter = mapTransportPeerId.begin(); iter != mapTransportPeerId.end();)
        {
            if (iter->second == listener)
            {
                mapTransportPeerId.erase(iter++);
            }
            else
            {
                iter++;
            }
        }
    }
} // namespace RTC