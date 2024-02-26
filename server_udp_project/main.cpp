//#include <iostream>
//#include <string>
//#include <cstring>
//#include <fstream>
//#include <filesystem>
//#include <boost/asio.hpp>
//#include "../../libRaptorQ-master/src/RaptorQ/RaptorQ_v1.hpp"
//#include "../fileStructure.pb.h"
#include "serverHandling.hpp"

#define PORT 12345

int main() {
    try {
        std::cout << "SERVER IS RUNNING..." << std::endl;
        boost::asio::io_context io_context;

        //std::vector<std::string> packetsVector;

        // creates a UDP socket binding it to the local UDP port 12345 using IPv4.
        boost::asio::ip::udp::socket socket(
                io_context,
                boost::asio::ip::udp::endpoint{boost::asio::ip::udp::v4(), PORT});

        boost::asio::ip::udp::endpoint client;

        std::string basePath = "/home/idan/Desktop/CLION_projects/UDP_NETWORKING/server_udp_project/files_from_client/";

        for (;;) {
            try {


                std::array<char, 128> recv_buf;
                boost::system::error_code ec;
                socket.async_receive_from(boost::asio::buffer(recv_buf), client,
                                          [&ec](const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
                                              ec = error;
                                          });

                // Wait for the operation to complete or timeout
                io_context.run_one();

                if (ec == boost::asio::error::operation_aborted) {
                    std::cerr << "Timed out waiting for packets." << std::endl;
                    break;
                }

                std::cout << "Received: " << std::string(recv_buf.data()) << std::endl;
                // Parse and process the received packet
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }



                // getting the first config packet
                FILE_STORAGE::ConfigPacket configPacket;
                for (int i = 0; i<5; ++i) {
                    try {
                        char receive_data[300]; // The max possible size // calculate the max data again
                        std::size_t bytesRead = socket.receive_from(boost::asio::buffer(receive_data), client);
                        configPacket.ParseFromString(std::string(receive_data, bytesRead));
                    } catch (std::exception &e) {
                        std::cerr << "Error occurred!\n" << e.what() << std::endl; // Unexpected packet
                    }
                }
//                FILE_STORAGE::ConfigPacket configPacket;
//                char receive_data[300];
//                std::size_t bytesRead = socket.receive_from(boost::asio::buffer(receive_data), client);
//                std::cout << "CONFIGURATION PACKET -> READS " << bytesRead << " BYTES." << std::endl;
//                configPacket.ParseFromString(std::string(receive_data, bytesRead));
                if (configPacket.type() == FILE_STORAGE::FileType::DIRECTORY)
                    std::filesystem::create_directory(basePath + configPacket.name());
                else {
                    std::string filePath = basePath + configPacket.name();
                    serverHandling::receivePacket(socket, configPacket.chunks(), configPacket.chunk_size(), filePath);
                }
            } catch (std::exception& e) {
                std::cerr << "Error occurred!\n" << e.what() << std::endl; // Unexpected packet
            }
        }
    }
    catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}


