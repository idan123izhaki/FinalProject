#include <iostream>
#include <boost/asio.hpp>
#include "fileHandler.hpp"

// paths
// /home/idan/Desktop/CLION_projects/files/f1
// /home/idan/Desktop/CLION_projects/files/dir1
// /home/idan/Desktop/CLION_projects/files/big.txt

#define PORT 12345

int main()
{
    std::vector<std::thread> threads;
    try {
        std::cout << "CLIENT IS RUNNING ..." << std::endl;
        boost::asio::io_context io_context;
        // creating a UDP socket
//        boost::asio::ip::udp::socket socket(io_context);
//        // opening a UDP socket and configuring it to use IPv4 addressing
//        socket.open(boost::asio::ip::udp::v4());
        UdpClient client(io_context, PORT);
        int inotify_fd = client.init_inotify_obj();
        for(;;)
        {
            // get and check the path from client
            std::string path = client.pathHandler();

            unsigned long chunkSize;
            std::cout << "Please enter the chunk size you want to split the file: " << std::endl;
            std::cin >> chunkSize;

            uint32_t symbol_size;
            std::cout << "Please enter the size of each symbol you want to send: " << std::endl;
            std::cin >> symbol_size;

            uint32_t overhead;
            std::cout << "Please enter the number of overhead packets: " << std::endl;
            std::cin >> overhead;

            client.directory_file_scanner(path, chunkSize, symbol_size, overhead, "", inotify_fd);

            // creating a thread to monitoring the current path
            std::thread t1([&client, inotify_fd, chunkSize, symbol_size, overhead] {
                client.monitorFunc(inotify_fd, chunkSize, symbol_size, overhead);
            });
            t1.detach();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}
