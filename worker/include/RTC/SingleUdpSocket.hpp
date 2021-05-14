#ifndef MS_RTC_SINGLE_UDP_SOCKET_HPP
#define MS_RTC_SINGLE_UDP_SOCKET_HPP

#include "common.hpp"

#include "RTC/UdpSocket.hpp"
#include <string>
#include <unordered_map>
namespace RTC
{
    class SingleUdpSocket : public RTC::UdpSocket::Listener
    {
    public:
        void OnUdpSocketPacketReceived(
          RTC::UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr) override;

    public:
        SingleUdpSocket(std::string& ip);
        ~SingleUdpSocket();
        void SetTransportByUserName(RTC::UdpSocket::Listener* listener, const std::string& name);
        void ChangeTransportByUserName(
          RTC::UdpSocket::Listener* listener, const std::string& old_name, const std::string& new_name);
        void SetTransportByPeerId(RTC::UdpSocket::Listener* listener, const std::string& name);
        uint16_t GetLocalPort();
        std::string GetLocalIp();
        void DeleteTransport(RTC::UdpSocket::Listener* listener);
    private:
        void GetPeerId(const struct sockaddr* remoteAddr, std::string& peer_id);
    private:
        // Passed by argument.
        RTC::UdpSocket* udpSocket;
        std::unordered_map<std::string, RTC::UdpSocket::Listener*> mapTransportPeerId;
        std::unordered_map<std::string, RTC::UdpSocket::Listener*> mapTransportName;
    };
} // namespace RTC

#endif