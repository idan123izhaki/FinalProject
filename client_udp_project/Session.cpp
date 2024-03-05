#include "Session.hpp"

Session::Session(std::string IP, unsigned short port, boost::asio::io_context& io_context, std::string path, uint32_t chunk_size, uint32_t symbol_size, uint32_t overhead)
        : socket(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::make_address(IP), port))
{
    this->IP = IP;
    this->port = port;
    //this->io_context = io_context; // does not working... think how to save the io_context for async sending and receive messages
    this->path = path;
    this->chunk_size = chunk_size;
    this->symbol_size = symbol_size;
    this->overhead = overhead;
}

void Session::createSession(std::string IP, unsigned short port, boost::asio::io_context& io_context, std::string path, uint32_t chunk_size, uint32_t symbol_size,
                       uint32_t overhead) {

    try {
        std::cout << "BEFORE CREATING SESSION INSTANCE!!!" << std::endl;
        Session session(IP, port, io_context, path, chunk_size, symbol_size, overhead);
        std::cout << "AFTER CREATING SESSION INSTANCE - SUCCESSFULLY!!!" << std::endl;

        io_context.run(); // checking this line- needs to start the async sending

    }
    catch(std::exception& e)
    {
        std::cerr << "error occurred in thread function: \n" << e.what() << std::endl;
    }
}

Session::~Session() {
    this->socket.close(); //only while the session removed by the user or something
}

