#include <iostream>
#include <fstream>
//#include <limits>
#include <random>
//#include <stdlib.h>
#include <vector>
#include "../libRaptorQ-master/src/RaptorQ/RaptorQ_v1_hdr.hpp"
//#include "../fileStructure.pb.h" // protoBuf file

#ifndef CLIENT_UDP_PROJECT_FECALGORITHM_HPP
#define CLIENT_UDP_PROJECT_FECALGORITHM_HPP
namespace RaptorQ = RaptorQ__v1;

class Fec{
public:
    // returns the min_symbols number
    static uint64_t getSymbolNum(uint64_t input_size, uint32_t symbol_size);
    // returns the block size after checking if valid
    static RaptorQ::Block_Size getBlockSize(uint64_t min_symbols);

    // returns vector of uint_8 (bytes) before sending
    static std::vector<uint8_t> createVector(uint32_t num1, uint32_t num2, std::vector<uint8_t> data);

    // encode static function
    static std::vector<std::pair<uint32_t, std::vector<uint8_t>>> encoder(
            const std::string& inputString, uint32_t symbol_size, uint32_t overhead);

    // decode static function
    static std::vector<uint8_t> decoder(RaptorQ::Block_Size block, uint32_t chunk_size, uint32_t symbol_size,
                               std::vector<std::pair<uint32_t, std::vector<uint8_t>>> received);
};

#endif //CLIENT_UDP_PROJECT_FECALGORITHM_HPP
