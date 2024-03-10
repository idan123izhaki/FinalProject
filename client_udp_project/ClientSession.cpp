#include "ClientSession.hpp"

ClientSession::ClientSession(std::string IP, unsigned short port, boost::asio::io_context& io_context_, std::string path, uint32_t chunk_size, uint32_t symbol_size, uint32_t overhead)
        : io_context(io_context_), socket(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::make_address(IP), port))
{
    this->IP = IP;
    this->port = port;
    this->path = path;
    this->chunk_size = chunk_size;
    this->symbol_size = symbol_size;
    this->overhead = overhead;
}


void ClientSession::start() {
    this->io_context.run();
}


void ClientSession::createSession(std::string IP, unsigned short port, boost::asio::io_context& io_context, std::string path, uint32_t chunk_size, uint32_t symbol_size,
                                  uint32_t overhead) {

    try {
        std::cout << "BEFORE CREATING SESSION INSTANCE!!!" << std::endl;
        ClientSession session(IP, port, io_context, path, chunk_size, symbol_size, overhead);
        std::cout << "AFTER CREATING SESSION INSTANCE - SUCCESSFULLY!!!" << std::endl;

        io_context.run(); // checking this line- needs to start the async sending

    }
    catch(std::exception& e)
    {
        std::cerr << "error occurred in thread function: \n" << e.what() << std::endl;
    }
}

ClientSession::~ClientSession() {
    this->socket.close(); // only while the session removed by the user or something
}

