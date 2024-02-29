//
// Created by idan on 2/27/24.
//
#include <iostream>
#include <boost/asio.hpp>
#include "fileHandler.hpp" // check which function there

#ifndef CLIENT_UDP_PROJECT_SESSION_HPP
#define CLIENT_UDP_PROJECT_SESSION_HPP


class Session {
    std::string IP, path;
    int inotify_fd;
    uint32_t chunk_size, symbol_size, overhead;
    unsigned short port;
    boost::asio::ip::udp::socket socket;
    boost::asio::io_context io_context;
    boost::asio::ip::udp::endpoint  remote_endpoint;
    //std::mutex mutex_socket; //mutex_structure; // see word document, point 3
    std::map<int, std::pair<std::string, std::string>> map_path; // for listening changes

public:
    Session(std::string IP, unsigned short port, std::string path, uint32_t chunk_size, uint32_t symbol_size, uint32_t overhead);
};


#endif //CLIENT_UDP_PROJECT_SESSION_HPP
