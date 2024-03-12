//#include <boost/asio.hpp>
//#include <iostream>
//#include <vector>
//#include <cstring> // For std::memcpy
//
//using boost::asio::ip::udp;
//
//int main() {
//    boost::asio::io_context io_context;
//
//    udp::socket socket(io_context, udp::endpoint(udp::v4(), 0)); // UDP socket with any available port
//
//    // Manually create an endpoint for localhost and port 1234
//    udp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 1234);
//
//    // Creating the vector
//    std::vector<uint8_t> result;
//    std::string message = "Hello, world!\nhow are you?";
////    uint32_t num1 = 555555511;
////
////    // Copy the bytes of num1 into the vector
////    result.resize(sizeof(uint32_t));
////    std::memcpy(result.data(), &num1, sizeof(uint32_t));
////
////    // Insert the message data into the vector
////    result.insert(result.end(), message.begin(), message.end());
//
//    for(int i=0; i<10; ++i)
//    {
//        socket.async_send_to(boost::asio::buffer(message), endpoint,
//                             [](const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
//                                 if (!error) {
//                                     std::cout << "Message sent successfully." << std::endl;
//                                 } else {
//                                     std::cerr << "Error sending message: " << error.message() << std::endl;
//                                 }
//                             });
//        std::cout << "ROUND: " << i << std::endl;
//    }
//    io_context.run();
//    for(int i = 0; i< 10; ++i)
//        std::cout << "hi!" << std::endl;
//
//    // Run the io_context event loop
//
//
//    return 0;
//}


#include <iostream>
#include <boost/asio.hpp>
#include "../fileStructure.pb.h" // protoBuf file
#include "fecAlgorithm.hpp" // fec algorithm file
#include <filesystem>
using boost::asio::ip::udp;


std::vector<uint8_t> createHeader(bool isRegular, uint32_t conReg, uint32_t fileId, uint64_t chunk_id=0, uint32_t symbol_id=0) {
    std::vector<uint8_t> header;

    header.resize(sizeof(uint32_t) * 2);

    std::memcpy(header.data(), &conReg, sizeof(uint32_t)); // adding the packet type- config ot regular
    std::memcpy(header.data() + sizeof(uint32_t), &fileId, sizeof(uint32_t));
    if (isRegular)
    {
        header.resize(sizeof(uint32_t) * 3);
        std::memcpy(header.data() + sizeof(uint64_t) , &chunk_id, sizeof(uint64_t)); // adding the packet type - config or regular
        std::memcpy(header.data() + sizeof(uint64_t) * 2, &symbol_id, sizeof(uint32_t));
    }
    std::cout <<" --- THE HEADER SIZE IS: " << header.size() << " BYTES." << std::endl;
    return header;
}

int main() {
    try {
        boost::asio::io_context io_context;

        // Create a socket
        udp::socket socket(io_context, udp::endpoint(udp::v4(), 0));

        // Define the endpoint to which the data will be sent
        udp::endpoint receiver_endpoint_1(boost::asio::ip::address::from_string("127.0.0.1"), 1234);
        udp::endpoint receiver_endpoint_2(boost::asio::ip::address::from_string("127.0.0.1"), 1234);



        FILE_STORAGE::ConfigPacket configPacket;
//        if (std::filesystem::is_directory())
//            configPacket.set_type(FILE_STORAGE::FileType::DIRECTORY);
//        else
//        {
            configPacket.set_type(FILE_STORAGE::FileType::DIRECTORY);
            configPacket.set_con_type((true) ? FILE_STORAGE::ContentType::BINARY : FILE_STORAGE::ContentType::TEXT);
            //std::uint64_t fileSize = std::filesystem::file_size(path);
            configPacket.set_chunks((10000 % 20) ? (10000/20) + 1 : 10000/20);
            configPacket.set_chunk_size(20);
            //configPacket.set_block_size(static_cast<uint32_t>(fec::getBlockSize(fec::getSymbolNum(
      //              20, 5))));
            configPacket.set_symbol_size(5);
            configPacket.set_overhead(4);
        //}
        configPacket.set_name("currentName");

        uint32_t typePacket = 100; // because this is a configuration packet- 200 if regular packet
        uint32_t configID = 0; // needs to be equal to the file ID
        std::string serialized_data = configPacket.SerializeAsString();

        std::vector<uint8_t> finalPacket = createHeader(false, typePacket, 1); // after this the packet is ready with relevant headers
        finalPacket.insert(finalPacket.end(), serialized_data.begin(), serialized_data.end());


        // Send the buffer to the receiver
        socket.send_to(boost::asio::buffer(finalPacket), receiver_endpoint_1);

        std::cout << "Sent " << finalPacket.size() << " bytes to "
                  << receiver_endpoint_1.address().to_string() << ":" << receiver_endpoint_1.port() << std::endl;

        // Send the buffer to the receiver
        socket.send_to(boost::asio::buffer(finalPacket), receiver_endpoint_1);

        std::cout << "Sent " << finalPacket.size() << " bytes to "
                  << receiver_endpoint_1.address().to_string() << ":" << receiver_endpoint_1.port() << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(0));

        socket.send_to(boost::asio::buffer(finalPacket), receiver_endpoint_1);


        std::cout << "Sent " << finalPacket.size() << " bytes to "
                  << receiver_endpoint_1.address().to_string() << ":" << receiver_endpoint_1.port() << std::endl;

        // Send the buffer to the receiver
        socket.send_to(boost::asio::buffer(finalPacket), receiver_endpoint_1);

        std::cout << "Sent " << finalPacket.size() << " bytes to "
                  << receiver_endpoint_1.address().to_string() << ":" << receiver_endpoint_1.port() << std::endl;
//        // Send the buffer to the receiver
//        socket.send_to(boost::asio::buffer(finalPacket), receiver_endpoint_1);
//
//        std::cout << "Sent " << finalPacket.size() << " bytes to "
//                  << receiver_endpoint_1.address().to_string() << ":" << receiver_endpoint_1.port() << std::endl;
//
//        // Send the buffer to the receiver
//        socket.send_to(boost::asio::buffer(finalPacket), receiver_endpoint_2);
//
//        std::cout << "Sent " << finalPacket.size() << " bytes to "
//                  << receiver_endpoint_2.address().to_string() << ":" << receiver_endpoint_2.port() << std::endl;

        socket.close();

        while(true){}
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

