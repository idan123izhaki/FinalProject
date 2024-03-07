//
// Created by idan on 3/5/24.
//

#include "ServerSession.hpp"

// init the static member
std::string ServerSession::basePath = "/home/idan/Desktop/CLION_projects/UDP_NETWORKING/server_udp_project/files_from_client/";

ServerSession::ServerSession(int session_number, boost::asio::io_context& io_context, unsigned short port)
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


                    // because using thread
                    std::vector<uint8_t> buffer_copy(recv_buffer.begin(), recv_buffer.begin() + bytes_transferred);
                    this->recv_buffer.assign(MAX_BUFFER_SIZE, 0); // after each receiving - reset all

                    // Process received data in a separate thread
                    std::thread([this,buffer_copy, bytes_transferred]() {
                        process_data(buffer_copy, bytes_transferred);
                    }).detach();

                    receive_packets();
                }
                else
                {
                    std::cerr << "There is an error while receiving data.. session number: " << this->session_number << std::endl;
                }
            });
}


void ServerSession::process_data(std::vector<uint8_t> recv_data, std::size_t bytes_transferred)
{
    try {
        recv_data.resize(bytes_transferred);
        uint32_t packetType, file_id;
        std::memcpy(&packetType, recv_data.data(), sizeof(uint32_t)); // reads first 4 bytes - configuration or regular
        if (packetType == CONFIG)
        {
            std::memcpy(&file_id, recv_data.data() + sizeof(uint32_t), sizeof(uint32_t));
            if (!isExist(file_id))
            {
                std::cout << "This is a new config packet!" << std::endl;
                std::vector<uint8_t> serialized_data(recv_data.begin() + sizeof(uint64_t), recv_data.end());
                handleConfigPacket(file_id, serialized_data);

            } else {
                std::cout << "This config packet is already exist... ignore it..." << std::endl;
            }
        }
        else if(packetType == REGULAR)
        {
            if (isExist(file_id)) // checking if the fileId existing in global vector
            {
                std::vector<uint8_t> packet_data(recv_data.begin() + sizeof(uint64_t), recv_data.end());
                handleRegularPacket(file_id, packet_data);
            }
            else {
                std::cerr << "Received before the config packet.. ignores the packet..." << std::endl;
            }
        }
    } catch(std::exception& e) {
        std::cerr << "error occurred in process_data function: " << e.what() << std::endl;
    }
}


bool ServerSession::isExist(uint32_t num) {
    for (auto & fileIteration : this->fileManagement) {
        if (fileIteration.first == num) {
            return true; // Number found
        }
    }
    return false; // Number not found
}


void ServerSession::handleConfigPacket(uint32_t fileId, std::vector<uint8_t>& recv_data) {
    FILE_STORAGE::ConfigPacket new_config;
    if(new_config.ParseFromArray(recv_data.data(), recv_data.size()))
    {
        if (new_config.type() == FILE_STORAGE::FileType::DIRECTORY)
        {
            std::filesystem::create_directory(basePath + new_config.name());
        }
        else if (new_config.type() == FILE_STORAGE::FileType::FILE)
        {
            this->mutex_fileManagement.lock();
            this->fileManagement.emplace_back(fileId, FileBuilder(fileId, basePath + new_config.name(), true,
                                                                            new_config.chunks(), new_config.block_size(),
                                                                            new_config.chunk_size(), new_config.symbol_size(),
                                                                            new_config.overhead())); // adding it into the global vector
            this->mutex_fileManagement.unlock();
        }
    }
    else {
        std::cerr << "Failed to parse serialized data. (in process_data function)." << std::endl;
    }
}

void ServerSession::handleRegularPacket(uint32_t fileId, std::vector<uint8_t>& packet_data) {
    uint64_t chunk_id;
    uint32_t symbol_id;
    std::memcpy(&chunk_id, packet_data.data(), sizeof(uint64_t));
    std::memcpy(&symbol_id, packet_data.data() + sizeof(uint64_t), sizeof(uint32_t));
    std::vector<uint8_t> symbol_raw(packet_data.begin() + sizeof(uint64_t) + sizeof(uint32_t), packet_data.end());

    std::pair<uint32_t,std::vector<uint8_t>> symbol_packet = std::make_pair(symbol_id, symbol_raw);
    this->fileManagement[fileId].second.add_symbol(chunk_id, symbol_packet);
}


ServerSession::~ServerSession() {
    std::cout << "SERVER CLOSED SESSION NUMBER: " << this->session_number  << "." << std::endl;
    this->socket.close();
}
