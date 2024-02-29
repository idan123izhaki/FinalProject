#include <iostream>
#include <boost/asio.hpp>
#include "fileHandler.hpp"

// paths
// /home/idan/Desktop/CLION_projects/files/f1
// /home/idan/Desktop/CLION_projects/files/dir1
// /home/idan/Desktop/CLION_projects/files/big.txt

//#define PORT 12345 // the server port

int sessionNumber = 1;

int main() {
    for(;;) {
        try {

                std::cout << "CLIENT IS RUNNING (now, session number " << sessionNumber << ")..." << std::endl;
                std::cout << "Hello, please enter an available port number (for session " << sessionNumber << "): ";
                ++sessionNumber;
                unsigned short port;
                std::cin >> port;

                std::string path = UdpClient::pathHandler();

                unsigned long chunkSize;
                std::cout << "Please enter the chunk size you want to split the file: " << std::endl;
                std::cin >> chunkSize;

                uint32_t symbol_size;
                std::cout << "Please enter the size of each symbol you want to send: " << std::endl;
                std::cin >> symbol_size;

                uint32_t overhead;
                std::cout << "Please enter the number of overhead packets: " << std::endl;
                std::cin >> overhead;

        }
        catch(std::exception& e)
        {

        }
    }
    return 0;
}