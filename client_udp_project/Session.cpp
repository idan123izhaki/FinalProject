//
// Created by idan on 2/27/24.
//

#include "Session.hpp"

Session::Session(std::string path, uint32_t chunk_size, uint32_t symbol_size, uint32_t overhead) : socket(io_context) {
    this->path = path;
    this->chunk_size = chunk_size;
    this->symbol_size = symbol_size;
    this->overhead = overhead;
    this->remote_endpoint = boost::asio::ip::udp::endpoint{boost::asio::ip::make_address("127.0.0.1"), 1234};
}
