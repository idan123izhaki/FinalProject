#include <iostream>
#include <boost/asio.hpp>

#ifndef CLIENT_UDP_PROJECT_SESSION_HPP
#define CLIENT_UDP_PROJECT_SESSION_HPP

class ClientSession {

public:
    ClientSession(int session_number, std::string IP, unsigned short port, boost::asio::io_context& io_context);
    void sendingPackets(std::vector<uint8_t> packet);
    //void startSending();
    int getSessionNumber() const;
    ~ClientSession();

    boost::asio::io_context& io_context;

private:
    int session_number;
    std::string destinationIP;
    unsigned short destinationPort;
    boost::asio::ip::udp::endpoint  remote_endpoint;
    boost::asio::ip::udp::socket socket;
};

#endif //CLIENT_UDP_PROJECT_SESSION_HPP
