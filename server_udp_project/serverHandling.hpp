#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <boost/asio.hpp>
#include "../../libRaptorQ-master/src/RaptorQ/RaptorQ_v1.hpp"
#include "../fileStructure.pb.h"

#ifndef SERVER_UDP_PROJECT_SERVERHANDING_HPP
#define SERVER_UDP_PROJECT_SERVERHANDING_HPP

class serverHandling{
public:

    static void receiveConfigPacket(const boost::system::error_code& error, std::size_t bytes_transferred,
                                    boost::asio::ip::udp::endpoint sender_endpoint,
                                    boost::asio::ip::udp::socket& socket,
                                    boost::asio::io_context& io_context);
    static void receivePacket(boost::asio::ip::udp::socket& socket, unsigned long packetsNumber, unsigned int chunkSize, std::string& finalPath);
    static void createFileAndWrite(std::vector<std::string>& contentVector, unsigned long packetsNumber, std::string& finalPath);
};

#endif //SERVER_UDP_PROJECT_SERVERHANDING_HPP
