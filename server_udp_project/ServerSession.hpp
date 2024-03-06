//
// Created by idan on 3/5/24.
//
#include <iostream>
#include <boost/asio.hpp>
#include "FileBuilder.hpp"

#ifndef SERVER_UDP_PROJECT_SERVERSESSION_HPP
#define SERVER_UDP_PROJECT_SERVERSESSION_HPP

#define MAX_BUFFER_SIZE 10000

enum PacketType {
    CONFIG = 100,
    REGULAR = 200
};

class ServerSession {
    int session_number;
    boost::asio::ip::udp::socket socket;
    boost::asio::ip::udp::endpoint sender_endpoint;
    std::vector<std::pair<uint32_t, FileBuilder>> fileIdObject; // contain all the unique identification of config/regular packets with the match FIleBuilder object.
    std::vector<uint8_t> recv_buffer;
    static std::string basePath;

    std::mutex mutex_fileIdObject, mutex_structure; // see word document, point 3

public:
    ServerSession(int session_number, boost::asio::io_context& io_context, unsigned short port);

    void receive_packets(); // async receiving of packets
    void process_data(std::vector<uint8_t> recv_data, std::size_t bytes_transferred);
    bool isExist(uint32_t num);

    void handleConfigPacket(uint32_t fileId, std::vector<uint8_t>& serialized_data);
    void handleRegularPacket();


    ~ServerSession();
};


#endif //SERVER_UDP_PROJECT_SERVERSESSION_HPP



////
//// Created by idan on 2/27/24.
////
//#include <iostream>
//#include <boost/asio.hpp>
////#include "fileHandler.hpp" // check which function there
//
//#ifndef CLIENT_UDP_PROJECT_SESSION_HPP
//#define CLIENT_UDP_PROJECT_SESSION_HPP
//
//
//class Session {
//    std::string IP, path;
//    unsigned short port;
//    int inotify_fd;
//    uint32_t chunk_size, symbol_size, overhead;
//    boost::asio::ip::udp::socket socket;
//    boost::asio::io_context io_context;
//    boost::asio::ip::udp::endpoint  remote_endpoint;
//    std::mutex mutex_socket, mutex_structure; // see word document, point 3
//    std::map<int, std::pair<std::string, std::string>> map_path; // for listening changes
//
//public:
//    Session(std::string IP, unsigned short port, boost::asio::io_context& io_context, std::string path, uint32_t chunk_size, uint32_t symbol_size, uint32_t overhead);
//    static void createSession(std::string IP, unsigned short port, boost::asio::io_context& io_context, std::string path, uint32_t chunk_size, uint32_t symbol_size, uint32_t overhead);
//    ~Session();
//};
//
//#endif //CLIENT_UDP_PROJECT_SESSION_HPP
