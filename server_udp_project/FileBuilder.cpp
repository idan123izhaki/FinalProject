//
// Created by idan on 2/27/24.
//

#include "FileBuilder.hpp"


FileBuilder::FileBuilder(uint32_t file_id, uint64_t chunks_number, uint32_t symbols_number, unsigned long chunk_size, uint32_t symbol_size,
                         uint32_t overhead) {
    this->file_id = file_id;
    this->chunks_number = chunks_number;
    this->symbols_number = symbols_number;
    this->chunk_size = chunk_size;
    this->symbol_size = symbol_size;
    this->overhead = overhead;
}

//adding the symbol to map
void FileBuilder::add_symbol(uint32_t chunk_id, std::pair<uint32_t,std::vector<uint8_t>> symbol_raw) {
    this->chunks_symbols_map[chunk_id]; //ensure the element existing
    this->chunks_symbols_map[chunk_id].resize(this->symbols_number);// if it at the same size - do nothing
    this->chunks_symbols_map[chunk_id].insert(this->chunks_symbols_map[chunk_id].begin() + symbol_raw.first, symbol_raw.second);
}

// adding at the chunk_id index the data after decoding
// each some time/some vector size -> write the data to the file
void FileBuilder::add_encode_data() {
    //decoded_info vector
}
