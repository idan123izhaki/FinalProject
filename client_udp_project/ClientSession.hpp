#include <iostream>
#include <boost/asio.hpp>

#ifndef CLIENT_UDP_PROJECT_SESSION_HPP
#define CLIENT_UDP_PROJECT_SESSION_HPP

class ClientSession {
    int session_number;
    std::string IP;
    unsigned short port;
    boost::asio::ip::udp::socket socket;
    boost::asio::io_context& io_context;
    boost::asio::ip::udp::endpoint  remote_endpoint;

public:
    ClientSession(int session_number, std::string IP, unsigned short port, boost::asio::io_context& io_context);
    void sendingPackets(std::vector<uint8_t> packet);
    void startSending();
    ~ClientSession();
};

#endif //CLIENT_UDP_PROJECT_SESSION_HPP
