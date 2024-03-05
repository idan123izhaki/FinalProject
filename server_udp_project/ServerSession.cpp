//
// Created by idan on 3/5/24.
//

#include "ServerSession.hpp"

ServerSession::ServerSession(boost::asio::io_context& io_context, unsigned short port, int session_number)
: socket(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
{
    this->session_number = session_number;
    this->recv_buffer.resize(MAX_BUFFER_SIZE);
    this->recv_buffer.assign(MAX_BUFFER_SIZE, 0); // after each receiving
    receive_packets();
}

void ServerSession::receive_packets()
{
    this->socket.async_receive_from(boost::asio::buffer(recv_buffer), this->sender_endpoint,
    [this] (const boost::system::error_code& error, std::size_t bytes_transferred) {
                if ((!error || error == boost::asio::error::message_size) && bytes_transferred > 0)
                {
                    std::cout << "Received " << bytes_transferred << " bytes from "
                              << sender_endpoint.address().to_string() << ":" << sender_endpoint.port() << std::endl;
                    std::string str (reinterpret_cast<const char*>(recv_buffer.data()));
                    std::cout << "-> received message successfully: ";
                    std::cout << str << std::endl;

                    // Process received data in a separate thread
                    std::thread([this, bytes_transferred]() {
                        process_data(recv_buffer, bytes_transferred);
                    }).detach();

                    receive_packets();
                }
                else
                {
                    std::cout << "There is an error while receiving data.. session number: " << this->session_number << std::endl;
                }
            });
}


void ServerSession::process_data(std::vector<uint8_t> recv_data, std::size_t bytes_transferred)
{
    recv_data.resize(bytes_transferred);
    uint32_t packetType, fileId;
    std::memcpy(&packetType, recv_data.data(), sizeof(uint32_t));
    if (packetType == CONFIG)
    {
        std::memcpy(&fileId, recv_data.data() + sizeof(uint32_t), sizeof(uint32_t));
        if (checkingRepetition(fileId))
        {
            std::cout << "THIS IS A NEW CONFIG PACKET!" << std::endl;

        } else {
            std::cout << "THIS CONFIG PACKET ALREADY EXIST..." << std::endl;
        }
    }
    else if(packetType == REGULAR)
    {
        handleRegularPacket(); // building new FileBuilder instance, and save it reference maybe in global vector or something with its id.
        // maybe create as a member of the session class a vector of pairs: <uint32_t fileId, FileBuilder fileInstance>.. need to think about it
    }
}

bool ServerSession::checkingRepetition(uint32_t num) {
    for (uint32_t x : this->FileId) {
        if (x == num) {
            return true; // Number found
        }
    }
    return false; // Number not found
}

void ServerSession::handleConfigPacket() {

}

void ServerSession::handleRegularPacket() {

}

ServerSession::~ServerSession() {
    std::cout << "SERVER CLOSED SESSION NUMBER: " << this->session_number  << "." << std::endl;
    this->socket.close();
}


