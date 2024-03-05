#include <boost/asio.hpp>
#include <iostream>
#include <vector>
#include <cstring> // For std::memcpy

using boost::asio::ip::udp;

int main() {
    boost::asio::io_context io_context;

    udp::socket socket(io_context, udp::endpoint(udp::v4(), 0)); // UDP socket with any available port

    // Manually create an endpoint for localhost and port 1234
    udp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 1234);

    // Creating the vector
    std::vector<uint8_t> result;
    std::string message = "Hello, world!\nhow are you?";
//    uint32_t num1 = 555555511;
//
//    // Copy the bytes of num1 into the vector
//    result.resize(sizeof(uint32_t));
//    std::memcpy(result.data(), &num1, sizeof(uint32_t));
//
//    // Insert the message data into the vector
//    result.insert(result.end(), message.begin(), message.end());

    for(int i=0; i<10; ++i)
    {
        socket.async_send_to(boost::asio::buffer(message), endpoint,
                             [](const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
                                 if (!error) {
                                     std::cout << "Message sent successfully." << std::endl;
                                 } else {
                                     std::cerr << "Error sending message: " << error.message() << std::endl;
                                 }
                             });
        std::cout << "ROUND: " << i << std::endl;
    }
    io_context.run();
    for(int i = 0; i< 10; ++i)
        std::cout << "hi!" << std::endl;

    // Run the io_context event loop


    return 0;
}
