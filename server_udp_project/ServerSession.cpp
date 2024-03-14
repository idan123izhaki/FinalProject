#include "ServerSession.hpp"

// init the static member
std::string ServerSession::basePath = "/home/idan/Desktop/CLION_projects/UDP_NETWORKING/server_udp_project/files_from_client/";

ServerSession::ServerSession(int session_number, boost::asio::io_context& io_context_, unsigned short port)
: io_context(io_context_), socket(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
{
    this->session_number = session_number;
    this->recv_buffer.resize(MAX_BUFFER_SIZE);
    this->recv_buffer.assign(MAX_BUFFER_SIZE, 0); // after each receiving
    std::cout << "new session creates--> session number " << session_number << std::endl;
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

                    uint32_t fileId;
                    memcpy(&fileId, recv_buffer.data()+sizeof(uint32_t), sizeof(uint32_t));
                    //std::cerr << "FILE ID = " << fileId << "." << std::endl;

                    std::string str(recv_buffer.begin(), recv_buffer.begin() + bytes_transferred);

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


void ServerSession::start() {
    this->io_context.run();
}


void ServerSession::process_data(std::vector<uint8_t> recv_data, std::size_t bytes_transferred)
{
    try {
        //recv_data.resize(bytes_transferred);
        uint32_t packetType, file_id;
        std::memcpy(&packetType, recv_data.data(), sizeof(uint32_t)); // reads first 4 bytes - configuration or regular
        std::memcpy(&file_id, recv_data.data() + sizeof(uint32_t), sizeof(uint32_t));
        std::vector<uint8_t> packet_data(recv_data.begin() + sizeof(uint64_t), recv_data.end());

        if (packetType == CONFIG)
        {
            std::lock_guard<std::mutex> lock(this->mutex_file_management);
            std::cout << "----------------- in 'process_data' function, config function, file id: " << file_id << std::endl;
            if (!isExist(file_id))
            {
                std::cout << "session number: " << this->session_number << " -> This is new config packet!" << std::endl;
                handleConfigPacket(file_id, packet_data);
            } else {
                std::cout << "This config packet is already exist... ignore it..." << std::endl;
            }
        }
        else if(packetType == REGULAR)
        {
            std::lock_guard<std::mutex> lock(this->mutex_file_management);
            std::cout << "in 'process_data' function, regular function, file id: " << file_id << std::endl;
            if (isExist(file_id)) // checking if the fileId existing in global vector
            {
                handleRegularPacket(file_id, packet_data);
            }
            else
            {
                std::lock_guard<std::mutex> lockPacketLost(this->mutex_lost_packets);
                this->regularPacketsLost[file_id]; // create if not exist
                this->regularPacketsLost[file_id].resize(this->regularPacketsLost[file_id].size() + 1); // resize the vector size by 1
                this->regularPacketsLost[file_id].push_back(packet_data); // pushing new lost packet
                std::cerr << "Received before the config packet.. save the packet in vector..." << std::endl;
            }
        }
        else
            std::cerr << "Error occurred to packet header..." << std::endl;
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

    bool mode, configDirectory = false;
    if(new_config.ParseFromArray(recv_data.data(), recv_data.size()))
    {
        if (new_config.type() == FILE_STORAGE::FileType::DIRECTORY)
        {
            std::filesystem::create_directory(basePath + new_config.name());
            configDirectory = true;
            std::cout << "-> create new directory: '" << new_config.name() << "'." << std::endl;
        }
        else if (new_config.type() == FILE_STORAGE::FileType::FILE) {
            std::cerr << "new_config: (symbols number)" << new_config.block_size() << std::endl;
            std::cerr << "new_config: (overhead number)" << new_config.overhead() << std::endl;
            mode = new_config.con_type() == FILE_STORAGE::ContentType::TEXT;
        }
        this->fileManagement.emplace_back(fileId,
                                          std::make_unique<FileBuilder>(fileId, basePath + new_config.name(), mode,
                                                                        new_config.chunks(),
                                                                        new_config.block_size(),
                                                                        new_config.chunk_size(),
                                                                        new_config.symbol_size(),
                                                                        new_config.overhead(),
                                                                        configDirectory)); // adding it into the global vector


        // closing the file object after 10 seconds
        std::thread t([this, fileId](){
            std::cout << "starting thread destroy file object (10 seconds)..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(10));
            this->closeFileObject(fileId);
        });
        t.detach();
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
    std::cout << "now in 'handleRegularPacket' function, packet number: " << fileId << "---" << chunk_id << "---" << symbol_id << "." << std::endl;
    std::vector<uint8_t> symbol_raw(packet_data.begin() + sizeof(uint64_t) + sizeof(uint32_t), packet_data.end());
    std::cerr << "symbol_raw.dataSize = " << symbol_raw.size() << std::endl;
    std::cerr << "symbol_raw.data = " << std::string(symbol_raw.begin(), symbol_raw.end()) << std::endl;
    std::cerr << "step 1..." << std::endl;
    std::pair<uint32_t,std::vector<uint8_t>> symbol_packet = std::make_pair(symbol_id, symbol_raw);
    std::cerr << "step 2..." << std::endl;
    this->fileManagement[fileId].second->add_symbol(chunk_id, symbol_packet);
    std::cerr << "step 3..." << std::endl;

}

void ServerSession::sendingLostPackets(uint32_t fileId){
    std::lock_guard<std::mutex> lock(this->mutex_lost_packets);
    //sending all the lost packets before closing the object file.
    auto& packets = this->regularPacketsLost[fileId];

    for (auto& vec : packets) {
        this->handleRegularPacket(fileId, vec);
    }
    packets.clear(); // after sending all - reset
}


// after X time-> close the file object-> destructor called.
void ServerSession::closeFileObject(uint32_t fileId)
{
    // before closing the file object - send all lost packets if exist
    this->sendingLostPackets(fileId);

    std::lock_guard<std::mutex> lock(this->mutex_file_management);
    for (auto it = fileManagement.begin(); it != fileManagement.end();) {
        if (it->first == fileId)
            it = fileManagement.erase(it);
        else
            ++it;
    }
}

ServerSession::~ServerSession() {
    std::cout << "SERVER CLOSED SESSION NUMBER: " << this->session_number  << "." << std::endl;
    this->socket.close();
}
