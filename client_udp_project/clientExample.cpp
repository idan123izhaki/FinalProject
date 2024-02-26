#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::udp;

int main() {
    boost::asio::io_context io_context;
    udp::socket socket(io_context, udp::endpoint(udp::v4(), 0)); // UDP socket with any available port

    udp::resolver resolver(io_context);
    udp::resolver::results_type endpoints = resolver.resolve(udp::v4(), "localhost", "1234");

    std::string message = "Hello, world!\nhow are you?";
    socket.async_send_to(boost::asio::buffer(message), *endpoints.begin(),
                         [](const boost::system::error_code& error, std::size_t /*bytes_transferred*/) {
                             if (!error) {
                                 std::cout << "Message sent successfully." << std::endl;
                             } else {
                                 std::cerr << "Error sending message: " << error.message() << std::endl;
                             }
                         });
    for(int i = 0; i< 1000; ++i)
        std::cout << "hi!" << std::endl;
    io_context.run();

    return 0;
}
