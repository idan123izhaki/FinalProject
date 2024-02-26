#include <iostream>
#include <vector>
#include <random>
#include "../../libRaptorQ-master/src/RaptorQ/RaptorQ_v1_hdr.hpp"
#include "../fileStructure.pb.h" // protoBuf file

namespace RaptorQ = RaptorQ__v1;

// Function to encode input string using RaptorQ encoding
bool encode(const std::string& inputString, std::vector<uint8_t>& encodedData) {
    // Convert input string to vector of uint8_t
    std::vector<uint8_t> input(inputString.begin(), inputString.end());

    // Symbol size and block size determination
    const uint16_t symbol_size = 4; // bytes
    RaptorQ::Block_Size block = RaptorQ::Block_Size::Block_10;

    // Initialize encoder
    RaptorQ::Encoder<typename std::vector<uint8_t>::iterator,
            typename std::vector<uint8_t>::iterator> enc(block, symbol_size);

    // Provide input to encoder
    if (enc.set_data(input.begin(), input.end()) != input.size()) {
        std::cout << "Could not give data to the encoder :(\n";
        return false;
    }

    // Run encoder
    if (!enc.compute_sync()) {
        std::cout << "Enc-RaptorQ failure! really bad!\n";
        return false;
    }

    // Get encoded data
    encodedData.clear();
    encodedData.insert(encodedData.end(), enc.data().begin(), enc.data().end());

    return true;
}

// Function to decode RaptorQ encoded data
bool decode(const std::vector<uint8_t>& encodedData, std::string& decodedString) {
    // Symbol size and block size determination
    const uint16_t symbol_size = 4; // bytes
    RaptorQ::Block_Size block = RaptorQ::Block_Size::Block_10;

    // Initialize decoder
    RaptorQ::Decoder<typename std::vector<uint8_t>::iterator,
            typename std::vector<uint8_t>::iterator> dec(block, symbol_size,
                                                         RaptorQ::Decoder<typename std::vector<uint8_t>::iterator,
                                                                 typename std::vector<uint8_t>::iterator>::Report::COMPLETE);

    // Provide encoded data to decoder
    if (dec.add_symbols(encodedData.begin(), encodedData.end()) != encodedData.size()) {
        std::cout << "Could not give encoded data to the decoder :(\n";
        return false;
    }

    // Decode data
    std::vector<uint8_t> decodedData;
    decodedData.resize(encodedData.size()); // Ensure enough space for decoded data
    auto decoded = dec.decode_bytes(decodedData.begin(), decodedData.end(), 0, 0);

    // Check if decoding was successful
    if (decoded.written != encodedData.size()) {
        std::cout << "Decoding failed: Incorrect number of bytes decoded\n";
        return false;
    }

    // Convert decoded data to string
    decodedString.assign(decodedData.begin(), decodedData.end());

    return true;
}

int main() {
    // Initialize random number generator
    std::random_device rd;
    std::mt19937_64 rnd(rd());

    // Example protobuf structure
    FILE_STORAGE::ConfigPacket configPacket;
    configPacket.set_type(FILE_STORAGE::FileType::DIRECTORY);
    configPacket.set_chunks(100);
    configPacket.set_chunk_size(20);
    configPacket.set_name("successful encode decode! aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

    // Serialize the data structure into string
    std::string inputString = configPacket.SerializeAsString();
    std::cout << "The size of the serialized data is: " << inputString.size() << std::endl;

    // Encode the input string
    std::vector<uint8_t> encodedData;
    if (encode(inputString, encodedData)) {
        std::cout << "Encoding succeeded!\n";

        // Decode the encoded data
        std::string decodedString;
        if (decode(encodedData, decodedString)) {
            std::cout << "Decoding succeeded!\n";
            std::cout << "Decoded string: " << decodedString << std::endl;
        } else {
            std::cout << "Decoding failed!\n";
        }
    } else {
        std::cout << "Encoding failed!\n";
    }

    return 0;
}
