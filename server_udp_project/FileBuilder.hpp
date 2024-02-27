//
// Created by idan on 2/27/24.
//
#include <iostream>
#include <map>
#include <vector>

#ifndef SERVER_UDP_PROJECT_FILEBUILDER_HPP
#define SERVER_UDP_PROJECT_FILEBUILDER_HPP

class FileBuilder {
    uint32_t file_id;
    uint64_t chunks_number;
    unsigned long chunk_size;
    uint32_t symbol_size, overhead, symbols_number;

    std::map <uint32_t, std::vector<std::vector<uint8_t>>> chunks_symbols_map; //key: chunk_id, value: vector of symbols
    std::vector <std::vector<uint8_t>> decoded_info; //each place representing a final chunk (all the symbols of the chunk after decoding).

public:
    FileBuilder(uint32_t file_id, uint64_t chunks_number, uint32_t symbols_number,
                unsigned long chunk_size, uint32_t symbol_size, uint32_t overhead);

    void add_symbol(uint32_t chunk_id, std::pair<uint32_t,std::vector<uint8_t>> symbol_raw);
    void add_encode_data();
};


#endif //SERVER_UDP_PROJECT_FILEBUILDER_HPP
