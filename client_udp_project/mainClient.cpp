#include <iostream>
//#include <boost/asio.hpp>
#include "FileManagement.hpp"

// paths
// /home/idan/Desktop/CLION_projects/files/f1
// /home/idan/Desktop/CLION_projects/files/dir1
// /home/idan/Desktop/CLION_projects/files/big.txt

//#define PORT 12345 // the server port

int sessionNumber = 1;
std::vector<Session> sessionVec;

int main() {
    for(;;) {
        try {
            std::cout << "CLIENT IS RUNNING (now, session number " << sessionNumber << ")..." << std::endl;
            //std::cout << "Hello, please enter an available port number (for session " << sessionNumber << "):" << std::endl;
            ++sessionNumber;
            unsigned short port;
            port = 12345; //std::cin >> port;

            std::string path = FileManagement::pathHandler();

            uint32_t chunk_size;
            //std::cout << "Please enter the chunk size you want to split the file: " << std::endl;
            chunk_size = 20; //std::cin >> chunk_size;

            uint32_t symbol_size;
            //std::cout << "Please enter the size of each symbol you want to send: " << std::endl;
            symbol_size = 5; //std::cin >> symbol_size;

            uint32_t overhead;
            //std::cout << "Please enter the number of overhead packets: " << std::endl;
            overhead = 5; //std::cin >> overhead;

            boost::asio::io_context io_context;

            // creating a thread - new session
            std::thread newSessionThread([port, path, &io_context, chunk_size, symbol_size, overhead]() {
                Session::createSession("127.0.0.1", port, io_context, path, chunk_size, symbol_size, overhead);
            });
            newSessionThread.detach();
        }
        catch(std::exception& e)
        {
            std::cerr << "error occurred: \n" << e.what() << std::endl;
        }
    }
    return 0;
}