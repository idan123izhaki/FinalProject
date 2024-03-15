#include "FileBuilder.hpp"

FileBuilder::FileBuilder(uint32_t file_id, std::string path, bool mode, uint64_t chunks_number, uint32_t symbols_number,
                         uint32_t chunk_size, uint32_t symbol_size, uint32_t overhead, bool configDirectory) :
                         runningFlag(true), thread(&FileBuilder::writeEachSecond, this) {
    this->file_id = file_id;
    this->path = path;
    this->mode = mode; // true->text, false->binary
    this->chunks_number = chunks_number;
    this->received_packets = 0;
    this->symbols_number = symbols_number;
    this->chunk_size = chunk_size;
    this->symbol_size = symbol_size;
    this->overhead = overhead;
    this->configDirectory = configDirectory;
    this->decoded_info.resize(this->chunks_number);
}

//constructor - for config packets represent a directory
//FileBuilder::FileBuilder(uint32_t file_id)
//{
//    this->file_id = file_id;
//    this->configDirectory = true;
//}

void FileBuilder::writeEachSecond() {
    if (!this->configDirectory) {
        while (runningFlag) {
            try {
                {
                    std::lock_guard<std::mutex> decoded_info_lock(this->decoded_info_mutex);
                    (this->mode) ? writeToTextFile() : writeToBinaryFile();
                }
                std::this_thread::sleep_for(std::chrono::seconds(5));
            } catch (const std::exception& e) {
                // Handle the exception here
                std::cerr << "Exception caught in writeEachSecond(): " << e.what() << std::endl;
                // Optionally, you can log the exception or take other actions.
            }
        }
    }
}


//adding the symbol to map
// std::map <uint32_t, std::vector<std::pair<uint32_t, std::vector<uint8_t>>>> chunks_symbols_map;
void FileBuilder::add_symbol(uint64_t chunk_id, std::pair<uint32_t,std::vector<uint8_t>>& symbol_raw) {
    std::cerr << "add_symbol function -> step 0..." << std::endl;
    std::lock_guard<std::mutex> lock(this->chunks_symbols_mutex);
    try{
        // Ensure chunk_id exists in the map and create it if it doesn't
        auto& symbols = this->chunks_symbols_map[chunk_id];
        std::cerr << "add_symbol function -> step 2..." << std::endl;
        symbols.resize(symbols_number + overhead); // if is the same size - do nothing
        std::cerr << "add_symbol function -> step 3..." << std::endl;
        std::cerr << symbol_raw.first << std::endl;
        symbols[symbol_raw.first] =  symbol_raw;
        std::cerr << "add_symbol function -> step 4..." << std::endl;
        this->received_packets++; // adding 1 to packet counter
        std::cerr << "adding the symbol successfully!" << std::endl;

        std::lock_guard<std::mutex> checkSymbolsNum(this->chunks_symbols_num_mutex);
        chunksSymbolsNum[chunk_id].push_back(symbol_raw.first);

        // adding check of if all the symbols+overhead arrived -> send to decoder and insert to the decode_vector using add_decode_data function
        if (chunksSymbolsNum[chunk_id].size() == this->symbols_number + this->overhead)
        {
            std::cerr << "at server side, before decoding, number of symbols: " << this->chunks_symbols_map[chunk_id].size() << std::endl;
            std::cerr << "symbol_number + overhead = " << this->symbols_number + this->overhead << std::endl;
            // try to decode the data
            std::cerr << "#############################" << std::endl;
            printPairs(this->chunks_symbols_map[chunk_id]);
            std::cerr << "\t********** try to decode the data" << std::endl;
            try{
                std::vector<uint8_t> output = Fec::decoder(Fec::getBlockSize(this->symbols_number), this->chunk_size, symbol_size, this->chunks_symbols_map[chunk_id]);
                this->add_decode_data(chunk_id, output); // at the X (index) chunk_id -> inserting the decoded data
                this->chunks_symbols_map.erase(chunk_id); // after decode all the symbols - done with this chunk, and delete it
                this->chunksSymbolsNum.erase(chunk_id); // done with this chunk, and delete it
            } catch(std::exception& e) {
                std::cerr << "Error occurred while trying decode data...\n" << e.what() << std::endl;
            }
        }
    }
    catch(std::exception& e)
    {
        std::cerr << "ERROR OCCURRED IN 'ADD SYMBOL' FUNCTION ->  " << e.what() << std::endl;
    }
}

//std::vector <std::vector<uint8_t>> decoded_info;
// adding at the chunk_id index the data after decoding// first id=0
// each some time/vector size -> write the data to the file
void FileBuilder::add_decode_data(uint64_t chunk_id, std::vector<std::uint8_t>& decoded_data) {
    try {
        //locking the decoded_info vector
        std::lock_guard<std::mutex> lock(this->decoded_info_mutex);
        //this->decoded_info[chunk_id].resize(decoded_data.size());
        this->decoded_info[chunk_id] = decoded_data;
    } catch (const std::exception& e) {
        std::cerr << "Exception caught in add_decode_data(): " << e.what() << std::endl;
    }
}


void FileBuilder::writeToTextFile() {
    try {
        std::ofstream file(this->path, std::ios::app); // append mode
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open text file for writing!" << std::endl;
            return;
        }
        std::cerr << "Open text file for writing!" << std::endl;
        for(uint64_t i = 0; i < this->chunks_number; ++i) {
            std::cerr << "this->decoded_info[" << i << "] : " << std::string(this->decoded_info[i].begin(), this->decoded_info[i].end()) << std::endl;
            if (!(this->decoded_info[i].empty())) {
                if (i == this->chunks_number - 1) {
                    auto nullPos = std::find(this->decoded_info[i].begin(), this->decoded_info[i].end(), '\0');
                    if (nullPos != this->decoded_info[i].end()) {
                        // Erase all elements from the null character
                        this->decoded_info[i].erase(nullPos, this->decoded_info[i].end());
                    }
                }
                file.seekp(i * this->chunk_size);
                for (uint8_t byte : this->decoded_info[i])
                    file.put(static_cast<char>(byte)); //write char by char

                // Deleting this item from vector after writing
            }
        }
        this->decoded_info.clear();
        this->decoded_info.resize(this->chunks_number);
        file.close();
    } catch (const std::exception& e) {
        // Handle the exception here
        std::cerr << "Exception caught in writeToTextFile(): " << e.what() << std::endl;
        // Optionally, you can log the exception or take other actions.
    }
}


void FileBuilder::writeToBinaryFile() {
    std::ofstream file(this->path, std::ios::binary | std::ios::app); // binary and append mode
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open binary file for writing!" << std::endl;
        return;
    }
    std::cerr << "Open binary file for writing!" << std::endl;
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
    this->decoded_info.clear();
    this->decoded_info.resize(this->chunks_number);
    file.close();
}

uint32_t FileBuilder::gettingLostPacketsNum() const {
    return (this->chunks_number * (this->symbols_number + this->overhead)) - this->received_packets; // calculate the lost packets number
}


void FileBuilder::printPairs(const std::vector<std::pair<uint32_t, std::vector<uint8_t>>>& pairs) {
    try {
        std::cout << std::endl << std::endl;
        std::lock_guard<std::mutex> lock(this->print_pairs_mutex);
        for (const auto& pair : pairs) {
            std::cout << "First item: " << pair.first << ", Second item: " <<
                      std::string(pair.second.begin(), pair.second.end()) << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught in printPairs(): " << e.what() << std::endl;
    }
}


void FileBuilder::writingBeforeClosing() {
    try {
        std::lock_guard<std::mutex> lock(this->chunks_symbols_mutex);
        std::cerr << "symbol_number + overhead = " << this->symbols_number + this->overhead << std::endl;
        std::cerr << "receive packets: " << this->received_packets << std::endl;
        for (auto& chunk_symbols_pair : this->chunks_symbols_map) {
            // iterates over all map (all chunks)
            // needs to decode the data as is
            try {
                std::cerr << "inside for loop (writing before closing) -> " << chunk_symbols_pair.second.size() << std::endl;
                printPairs(chunk_symbols_pair.second);

                std::vector<uint8_t> output = Fec::decoder(Fec::getBlockSize(this->symbols_number), this->chunk_size, symbol_size, chunk_symbols_pair.second);
                std::lock_guard<std::mutex> decoded_info_lock(this->decoded_info_mutex);
                this->decoded_info[chunk_symbols_pair.first] = output; // at the X (index) chunk_id -> inserting the decoded data
                this->chunks_symbols_map.erase(chunk_symbols_pair.first); // after decode all the symbols - done with this chunk, and delete it
            } catch(std::exception& e) {
                std::cerr << "Error occurred while trying to decode data...\n" << e.what() << std::endl;
            }
        }
        std::lock_guard<std::mutex> decoded_info_lock(this->decoded_info_mutex);
        (this->mode) ? writeToTextFile() : writeToBinaryFile();
    } catch (const std::exception& e) {
        // Handle the exception here
        std::cerr << "Exception caught in writingBeforeClosing(): " << e.what() << std::endl;
        // Optionally, you can log the exception or take other actions.
    }
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
