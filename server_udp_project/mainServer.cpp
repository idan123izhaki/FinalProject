#include "FileBuilder.hpp"
#include <boost/asio.hpp>
#include "fecAlgorithm.hpp" // fec algorithm
#include "../fileStructure.pb.h" // protocol buffet

int sessionNumber = 1;
std::string basePath = "/home/idan/Desktop/CLION_projects/UDP_NETWORKING/server_udp_project/files_from_client/";

int main() {

        for(;;)
        {
            try {
                std::cout << "SERVER IS RUNNING (now, session number " << sessionNumber << ")..." << std::endl;
                std::cout << "Hello, please enter an available port number (for session " << sessionNumber << "): ";
                ++sessionNumber;
                unsigned short port;
                std::cin >> port;
                std::cout << "Session number " << sessionNumber << " on port number " << port << std::endl;

                // starting the session at separate thread
                std::thread sessionThread([&]() {
                    // needs to create here a new session object
                    //FileBuilder::sessionHandling(port);
                });

                sessionThread.detach();
            }
            catch (std::exception& e){
                std::cout << "error occurred -> \n" << e.what() << std::endl;
            }
        }

    return 0;
}
