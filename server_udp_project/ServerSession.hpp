#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <chrono>
#include "FileBuilder.hpp"

#ifndef SERVER_UDP_PROJECT_SERVERSESSION_HPP
#define SERVER_UDP_PROJECT_SERVERSESSION_HPP

#define MAX_BUFFER_SIZE 10000
#define TIME_OUT 100 // (in seconds) the time till the file object dead


enum PacketType {
    CONFIG = 100,
    REGULAR = 200
};

class ServerSession {
    int session_number;
    boost::asio::io_context& io_context;
    boost::asio::ip::udp::socket socket;
    boost::asio::ip::udp::endpoint sender_endpoint;

    std::map<uint32_t, std::unique_ptr<FileBuilder>> fileManagement; // contain all the unique identification of config/regular packets with the match FIleBuilder object.

    std::map<uint32_t, std::vector<std::vector<uint8_t>>> regularPacketsLost;

    std::vector<uint8_t> recv_buffer;
    static std::string basePath;

    std::mutex mutex_file_management, mutex_lost_packets; // see word document, point 3

public:
    ServerSession(int session_number, boost::asio::io_context& io_context, unsigned short port);

    void receive_packets(); // async receiving of packets

    void start();

    void process_data(std::vector<uint8_t> recv_data, std::size_t bytes_transferred);
    bool isExist(uint32_t num);

    void handleConfigPacket(uint32_t fileId, std::vector<uint8_t>& serialized_data);
    void handleRegularPacket(uint32_t fileId, std::vector<uint8_t>& packet_data);

    void sendingLostPackets(uint32_t fileId);
    void closeFileObject(uint32_t fileId);
    ~ServerSession();
};


#endif //SERVER_UDP_PROJECT_SERVERSESSION_HPP