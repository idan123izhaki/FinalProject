#include "FileBuilder.hpp"

FileBuilder::FileBuilder(uint32_t file_id, std::string path, bool mode, uint64_t chunks_number, uint32_t symbols_number,
                         uint32_t chunk_size, uint32_t symbol_size, uint32_t overhead) : runningFlag(true), thread(&FileBuilder::writeEachSecond, this) {
    this->file_id = file_id;
    this->path = path;
    this->mode = mode; // true->text, false->binary
    this->chunks_number = chunks_number;
    this->received_packets = 0;
    this->symbols_number = symbols_number;
    this->chunk_size = chunk_size;
    this->symbol_size = symbol_size;
    this->overhead = overhead;
    this->decoded_info.resize(this->chunks_number);
}

//constructor - for config packets represent a directory
FileBuilder::FileBuilder(uint32_t file_id)
{
    this->file_id = file_id;
    this->configDirectory = true;
}

void FileBuilder::writeEachSecond(){
    while(runningFlag)
    {
        {
            std::lock_guard<std::mutex> lock(this->decoded_info_mutex);
            (this->mode) ? writeToTextFile() : writeToBinaryFile();
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

//adding the symbol to map
// std::map <uint32_t, std::vector<std::pair<uint32_t, std::vector<uint8_t>>>> chunks_symbols_map;
void FileBuilder::add_symbol(uint32_t chunk_id, std::pair<uint32_t,std::vector<uint8_t>>& symbol_raw) {
    this->chunks_symbols_map[chunk_id]; //ensure the element existing
    this->chunks_symbols_map[chunk_id].resize(this->chunks_symbols_map[chunk_id].size() + 1);// if it at the same size - do nothing
    this->chunks_symbols_map[chunk_id].insert(this->chunks_symbols_map[chunk_id].begin() + symbol_raw.first, symbol_raw);
    this->received_packets++; // adding 1 to packet counter
    // adding check of if all the symbols+overhead arrived -> send to decoder and insert to the decode_vector using add_decode_data function
    if (this->chunks_symbols_map[chunk_id].size() == this->symbols_number + this->overhead)
    {
        // try to decode the data
        try{
            std::vector<uint8_t> output = Fec::decoder(Fec::getBlockSize(this->symbols_number), this->chunk_size, symbol_size, this->chunks_symbols_map[chunk_id]);
            std::lock_guard<std::mutex> lock(this->decoded_info_mutex);
            this->decoded_info[chunk_id] = output; // at the X (index) chunk_id -> inserting the decoded data
            this->chunks_symbols_map.erase(chunk_id); // after decode all the symbols - done with this chunk, and delete it
        } catch(std::exception& e) {
            std::cerr << "Error occurred while trying decode data...\n" << e.what() << std::endl;
        }
    }
}

//std::vector <std::vector<uint8_t>> decoded_info;
// adding at the chunk_id index the data after decoding// first id=0
// each some time/vector size -> write the data to the file
void FileBuilder::add_decode_data(uint32_t chunk_id, std::vector<std::uint8_t>& decoded_data) {
    //decoded_info vector
    std::lock_guard<std::mutex> lock(this->decoded_info_mutex);
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

            //deleting this item from vector after writing
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

uint32_t FileBuilder::gettingLostPacketsNum() const {
    return (this->chunks_number * (this->symbols_number + this->overhead)) - this->received_packets; // calculate the lost packets number
}

void FileBuilder::writingBeforeClosing() {
    for (auto& chunk_symbols_pair : this->chunks_symbols_map)
    {
        // iterates over all map
        // needs to decode the data as is
        try {
            std::vector<uint8_t> output = Fec::decoder(Fec::getBlockSize(this->symbols_number), this->chunk_size, symbol_size, chunk_symbols_pair.second);
            std::lock_guard<std::mutex> lock(this->decoded_info_mutex);
            this->decoded_info[chunk_symbols_pair.first] = output; // at the X (index) chunk_id -> inserting the decoded data
            this->chunks_symbols_map.erase(chunk_symbols_pair.first); // after decode all the symbols - done with this chunk, and delete it
        } catch(std::exception& e) {
            std::cerr << "Error occurred while trying decode data...\n" << e.what() << std::endl;
        }
    }
    std::lock_guard<std::mutex> lock(this->decoded_info_mutex);
    (this->mode) ? writeToTextFile() : writeToBinaryFile();
}


FileBuilder::~FileBuilder()
{
    if(!this->configDirectory)
    {
        this->runningFlag = false;
        if (thread.joinable())
            thread.join();
        writingBeforeClosing();// final writing - at the last time
        chunks_symbols_map.clear(); // maybe not deleting it, instead - each time i writing a packet -> deleting it from both map and vector
        decoded_info.clear(); // maybe not deleting it, instead - each time i writing a packet -> deleting it from both map and vector
    }
    std::cout << "(from FileBuilder class) The file: '" << this->path << "' ->  is dead." << std::endl;
}
