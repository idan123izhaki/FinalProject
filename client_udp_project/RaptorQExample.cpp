#include <iostream>
#include "../../libRaptorQ-master/src/RaptorQ/RaptorQ_v1_hdr.hpp"
#include "../fileStructure.pb.h" // protoBuf file

// Rename the main namespace for ease of use
namespace RaptorQ = RaptorQ__v1;

int main() {

    FILE_STORAGE::ConfigPacket configPacket;

    configPacket.set_type(FILE_STORAGE::FileType::DIRECTORY);
    configPacket.set_chunks(100);
    configPacket.set_chunk_size(20);
    configPacket.set_name("successful encode decode!");


    // adding this lines - updating the code with using now the libRaptorQ
    std::string serializedData = configPacket.SerializeAsString();

    // FEC parameters
    RaptorQ::Block_Size block = RaptorQ::Block_Size::Block_10;
    const uint16_t symbol_size = 4; // Change this based on your requirements
    const uint8_t overhead = 4;     // Adjust as needed

    // Initialize encoder
    RaptorQ::Encoder<std::string::iterator, std::string::iterator> encoder(block, symbol_size);

    // Set the serialized data to the encoder
    if (encoder.set_data(serializedData.begin(), serializedData.end()) != serializedData.size()) {
        std::cout << "Could not give data to the encoder :(\n";
        return -1;
    }

    // Run the encoder
    if (!encoder.compute_sync()) {
        std::cout << "Encoder failure!\n";
        return -1;
    }

    // Transmit the symbols, drop some based on drop_probability
    // (This part is not needed in your real-world application)
    // ...

    // Initialize decoder
    RaptorQ::Decoder<std::string::iterator, std::string::iterator> decoder(block, symbol_size, RaptorQ::Decoder<std::string::iterator, std::string::iterator>::Report::COMPLETE);

    // Receive and add the received symbols to the decoder
    // (This part is not needed in your real-world application)
    // ...

    // Signal the end of input to the decoder
    decoder.end_of_input(RaptorQ::Fill_With_Zeros::NO);

    // Run the decoder
    auto decodeResult = decoder.wait_sync();
    if (decodeResult.error != RaptorQ::Error::NONE) {
        std::cout << "Decoder couldn't decode.\n";
        return -1;
    }

    // Retrieve the decoded data
    std::string decodedData(serializedData.size(), 0);
    auto decodedIterator = decodedData.begin();
    auto decodedBytes = decoder.decode_bytes(decodedIterator, decodedData.end(), 0, 0);

    // Check if decoding was successful
    if (decodedBytes.written != serializedData.size()) {
        std::cout << "Decoding failure. Mismatched sizes.\n";
        return -1;
    }

    // Compare the original and decoded data
    if (serializedData != decodedData) {
        std::cout << "Decoded data does not match the original.\n";
        return -1;
    }

    std::cout << "Encoding and decoding successful!\n";

    return 0;
}
