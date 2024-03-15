#include <iostream>
//#include <boost/asio.hpp>
#include "FileManagement.hpp"

// paths
// /home/idan/Desktop/CLION_projects/files/f1
// /home/idan/Desktop/CLION_projects/files/dir1
// /home/idan/Desktop/CLION_projects/files/big.txt


int main() {
    int sessionNumber = 1;
    bool clientRunning = true;
    boost::asio::io_context io_context;

    while(clientRunning)
    {
        try {
            std::cout << "CLIENT IS RUNNING (now, session number " << sessionNumber << ")..." << std::endl;
            std::cout << "Hello, please enter an available port number (for session " << sessionNumber << "):" << std::endl;
            unsigned short port;
            std::cin >> port;

            std::string path = FileManagement::pathHandler();
            //std::string path = "/home/idan/Desktop/CLION_projects/files/dir2";

            uint32_t chunk_size;
            //std::cout << "Please enter the chunk size you want to split the file: " << std::endl;
            chunk_size = 20; //std::cin >> chunk_size;

            uint32_t symbol_size;
            //std::cout << "Please enter the size of each symbol you want to send: " << std::endl;
            symbol_size = 3; //std::cin >> symbol_size;

            uint32_t overhead;
            //std::cout << "Please enter the number of overhead packets: " << std::endl;
            overhead = 5; //std::cin >> overhead;

            std::cout << "trying create ClientSession instance.." << std::endl;

            // creating a thread - new session
            std::thread newSessionThread([sessionNumber, port, path, &io_context, chunk_size, symbol_size, overhead]() {
                //std::cout << "step 1" << std::endl;
                std::unique_ptr<ClientSession> new_session = std::make_unique<ClientSession>(sessionNumber, "127.0.0.1", port, io_context);
                //std::cout << "step 2" << std::endl;
                FileManagement new_file_manage(std::move(new_session), path, chunk_size, symbol_size, overhead);
                //std::cout << "step 3" << std::endl;
                new_file_manage.startSending();
                //std::cout << "step 4" << std::endl;
            });
            newSessionThread.detach();

            ++sessionNumber;
        }
        catch(std::exception& e)
        {
            std::cerr << "error occurred: \n" << e.what() << std::endl;
        }
    }
    return 0;
}
