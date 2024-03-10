#include "FileBuilder.hpp"
#include <boost/asio.hpp>
#include "fecAlgorithm.hpp" // fec algorithm
#include "../fileStructure.pb.h" // protocol buffer
#include "ServerSession.hpp"

std::string basePath = "/home/idan/Desktop/CLION_projects/UDP_NETWORKING/server_udp_project/files_from_client/";

int main() {

    int sessionNumber = 1;
    bool serverRunning = true;
    boost::asio::io_context io_context;

    while(serverRunning)
    {
        try {
            std::cout << "SERVER IS RUNNING (now, session number " << sessionNumber << ")..." << std::endl;
            std::cout << "Hello, please enter an available port number (for session " << sessionNumber << "): ";
            unsigned short port;
            std::cin >> port;
            std::cout << "ClientSession number " << sessionNumber << " on port number " << port << std::endl;

            // starting the session at separate thread
            std::thread sessionThread([&]() {
                // needs to create here a new session object
                ServerSession new_session (sessionNumber, io_context, port);
                new_session.start();
            });
            sessionThread.detach();
            ++sessionNumber;
        }
        catch (std::exception& e){
            std::cout << "error occurred -> \n" << e.what() << std::endl;
        }
    }

    return 0;
}
