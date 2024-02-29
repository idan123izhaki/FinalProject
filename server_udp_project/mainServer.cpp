//
// Created by idan on 2/29/24.
//

#include "FileBuilder.hpp"
#include <boost/asio.hpp>
#include "../../libRaptorQ-master/src/RaptorQ/RaptorQ_v1.hpp" // fec algorithm
#include "../fileStructure.pb.h" // protocol buffet

#define MAX_BUFFER_SIZE 10000

int sessionNumber = 1;

int main() {
    try {
        for(;;)
        {
            std::cout << "SERVER IS RUNNING (now, session number " << sessionNumber << ")..." << std::endl;
            std::cout << "Hello, please enter an available port number (for session " << sessionNumber << "): ";
            ++sessionNumber;
            int port;
            std::cin >> port;

            // creates a UDP socket binding it to the local UDP port 12345 using IPv4.
            boost::asio::io_context io_context;
            boost::asio::ip::udp::socket socket(
                    io_context,
                    boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), PORT));

            boost::asio::ip::udp::endpoint client_endpoint;

            std::string basePath = "/home/idan/Desktop/CLION_projects/UDP_NETWORKING/server_udp_project/files_from_client/";

            //receiving packets
            std::vector<uint8_t> received_buffer(MAX_BUFFER_SIZE);

            socket.async_receive_from(boost::asio::buffer(received_buffer), client_endpoint, FileBuilder::receiveHandling);
        }



    }
    catch (std::exception& e){
        std::cout << "error occurred -> \n" << e.what() << std::endl;
    }
    return 0;
}
