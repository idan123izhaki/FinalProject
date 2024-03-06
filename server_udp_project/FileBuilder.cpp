#include "FileBuilder.hpp"

FileBuilder::FileBuilder(uint32_t file_id, std::string path, bool mode, uint64_t chunks_number, uint32_t symbols_number,
                         uint32_t chunk_size, uint32_t symbol_size, uint32_t overhead) {
    this->file_id = file_id;
    this->path = path;
    this->mode = mode;
    this->chunks_number = chunks_number;
    this->received_packets = 0;
    this->symbols_number = symbols_number;
    this->chunk_size = chunk_size;
    this->symbol_size = symbol_size;
    this->overhead = overhead;
    this->decoded_info.resize(this->chunks_number);
}

//adding the symbol to map
void FileBuilder::add_symbol(uint32_t chunk_id, std::pair<uint32_t,std::vector<uint8_t>> symbol_raw) {
    this->chunks_symbols_map[chunk_id]; //ensure the element existing
    this->chunks_symbols_map[chunk_id].resize(this->symbols_number + this->overhead);// if it at the same size - do nothing
    this->chunks_symbols_map[chunk_id].insert(this->chunks_symbols_map[chunk_id].begin() + symbol_raw.first, symbol_raw.second);
    this->received_packets++; // adding 1 to packet counter
}


//std::vector <std::vector<uint8_t>> decoded_info;
// adding at the chunk_id index the data after decoding
// each some time/vector size -> write the data to the file
void FileBuilder::add_decode_data(uint32_t chunk_id, std::vector<std::uint8_t> decoded_data) {
    //decoded_info vector
    this->decoded_info[chunk_id].resize(decoded_data.size());
    this->decoded_info[chunk_id] = decoded_data;
}

void FileBuilder::writeToTextFile() {
    std::ofstream file(this->path, std::ios::app); // append mode
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open text file for writing!" << std::endl;
        return;
    }

    for(uint64_t i = 0; i < this->chunks_number; ++i)
    {
        if (!(this->decoded_info[i].empty()))
        {
            file.seekp(i * this->chunk_size);
            for (uint8_t byte : this->decoded_info[i])
                file.put(static_cast<char>(byte)); //write char by char

            //deleting this item from vector
            this->decoded_info.erase(this->decoded_info.begin() + i);
        }
    }
    file.close();
}

void FileBuilder::writeToBinaryFile() {
    std::ofstream file(this->path, std::ios::binary | std::ios::app); // binary and append mode
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open binary file for writing!" << std::endl;
        return;
    }

    for(uint64_t i = 0; i < this->chunks_number; ++i)
    {
        if (!(this->decoded_info[i].empty()))
        {
            file.seekp(i * this->chunk_size);
            file.write(reinterpret_cast<const char*>(this->decoded_info[i].data()), this->decoded_info[i].size()); // write binary data

            //deleting this item from vector
            this->decoded_info.erase(this->decoded_info.begin() + i);
        }
    }
    file.close();
}

uint32_t FileBuilder::gettingLostPacketsNum() const{
    return (this->chunks_number * (this->symbols_number + this->overhead)) - this->received_packets; // calculate the lost packets number
}

//void FileBuilder::receiveHandling(const boost::system::error_code& error, std::size_t bytes_transferred) {
//    if (!error) {
//        std::cout << "Received " << bytes_transferred << " bytes." << std::endl;
//    } else {
//        std::cerr << "Error: " << error.message() << std::endl;
//    }
//}

// receive the packets from this function, and route them according the packet type
// need to think how ot combine a time-out in this program..?
// this function also receiving the packets using async_receive_from function
//void FileBuilder::sessionHandling(unsigned short port) {
//    try{
//
//        // creates a UDP socket binding it to the local UDP port 12345 using IPv4.
//        boost::asio::io_context io_context;
//        boost::asio::ip::udp::socket socket(
//                io_context,
//                boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));
//
//        boost::asio::ip::udp::endpoint client_endpoint;
//
//        //receiving packets
//        std::vector<uint8_t> received_buffer(MAX_BUFFER_SIZE);
//
//        for(;;) // check if this loop is necessary
//        {
//            try{
//
//                socket.async_receive_from(boost::asio::buffer(received_buffer),
//                                          client_endpoint,
//                                          FileBuilder::receiveHandling);
//                std::cout << "at the sessionHandling function!" << std::endl;
//                io_context.run();
//
//            } catch(std::exception& e){
//                std::cout << "ERROR OCCURRED" << e.what() << std::endl;
//            }
//
//        }
//
//    } catch (std::exception& e){
//        std::cout << "ERROR OCCURRED -> " << e.what() << std::endl;
//    }
//}

FileBuilder::~FileBuilder() {
    chunks_symbols_map.clear(); // maybe not deleting it, instead- each time i writing a packet -> deleting it from both map and vector
    decoded_info.clear(); // maybe not deleting it, instead- each time i writing a packet -> deleting it from both map and vector
    std::cout << "The file dead.";
}

