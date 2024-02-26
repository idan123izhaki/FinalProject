#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::udp;

const int max_length = 1024;

void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, char* data) {
    if (!error) {
        std::cout << "Received message: " << data << std::endl;
    } else {
        std::cerr << "Error receiving message: " << error.message() << std::endl;
    }
}

int main() {
    boost::asio::io_context io_context;
    udp::socket socket(io_context, udp::endpoint(udp::v4(), 1234)); // UDP socket bound to port 1234

    char data[max_length];
    socket.async_receive(boost::asio::buffer(data, max_length),
                         [&socket, &data](const boost::system::error_code& error, std::size_t bytes_transferred) {
                             handle_receive(error, bytes_transferred, socket, data);
                         });

    io_context.run();
    return 0;
}
