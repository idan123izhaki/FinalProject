//#include <boost/asio.hpp>
//#include <iostream>
//
//using boost::asio::ip::udp;
//
//const int header_length = 4;
//const int max_length = 10000;
//
//void handle_receive_header(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, char* data, udp::endpoint& sender_endpoint);
//
//void handle_receive_data(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, char* data, udp::endpoint& sender_endpoint);
//
//int main() {
//    boost::asio::io_context io_context;
//    udp::socket socket(io_context, udp::endpoint(udp::v4(), 1234)); // UDP socket bound to port 1234
//
//    char header_data[header_length];
//    udp::endpoint sender_endpoint;
//    socket.async_receive_from(boost::asio::buffer(header_data, header_length), sender_endpoint,
//                              [&socket, &header_data, &sender_endpoint](const boost::system::error_code& error, std::size_t bytes_transferred) {
//                                  handle_receive_header(error, bytes_transferred, socket, header_data, sender_endpoint);
//                              });
//
//    io_context.run();
//    std::cout << "printing line!" << std::endl;
//    return 0;
//}
//
//void handle_receive_header(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, char* data, udp::endpoint& sender_endpoint) {
//    if (!error) {
//        // Convert the received header data to uint32_t
//        uint32_t header = *reinterpret_cast<uint32_t*>(data);
//
//        if (header > 1111111111) {
//            // Read 400 bytes
//            char data_buffer[400];
//            socket.async_receive_from(boost::asio::buffer(data_buffer, 400), sender_endpoint,
//                                      [&socket, &data_buffer, &sender_endpoint](const boost::system::error_code& error, std::size_t bytes_transferred) {
//                                          handle_receive_data(error, bytes_transferred, socket, data_buffer, sender_endpoint);
//                                      });
//        } else if (header < 1111111111) {
//            // Read 10000 bytes
//            char data_buffer[max_length];
//            socket.async_receive_from(boost::asio::buffer(data_buffer, max_length), sender_endpoint,
//                                      [&socket, &data_buffer, &sender_endpoint](const boost::system::error_code& error, std::size_t bytes_transferred) {
//                                          handle_receive_data(error, bytes_transferred, socket, data_buffer, sender_endpoint);
//                                      });
//        } else {
//            std::cerr << "Unknown header: " << header << std::endl;
//        }
//    } else {
//        std::cerr << "Error receiving header: " << error.message() << std::endl;
//    }
//}
//
//void handle_receive_data(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, char* data, udp::endpoint& sender_endpoint) {
//    if (!error) {
//        // Process received data based on the header and sender endpoint
//        // Example: Print the received data along with sender information
//        std::cout << "Received " << bytes_transferred << " bytes from " << sender_endpoint << ": " << std::string(data+4, bytes_transferred) << std::endl;
//    } else {
//        std::cerr << "Error receiving data: " << error.message() << std::endl;
//    }
//}



#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::udp;

const int max_packet_length = 10000; // max_size_length for any-data

enum Header
{   config = 0,
    regular = 1
};

void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, uint8_t* data, udp::endpoint& sender_endpoint, boost::asio::io_context& io_context);

//void handle_receive_data(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, char* data, udp::endpoint& sender_endpoint, boost::asio::io_context& io_context);

int main() {
    boost::asio::io_context io_context;
    udp::socket socket(io_context, udp::endpoint(udp::v4(), 1234)); // UDP socket bound to port 1234

    uint8_t packet[max_packet_length];
    udp::endpoint sender_endpoint;

    // Start the initial async receive operation
    socket.async_receive_from(boost::asio::buffer(packet, max_packet_length), sender_endpoint,
                              [&socket, &packet, &sender_endpoint, &io_context](const boost::system::error_code& error, std::size_t bytes_transferred) {
                                  handle_receive(error, bytes_transferred, socket, packet, sender_endpoint, io_context);
                              });

    // Create a work object to keep the io_context alive
    //boost::asio::io_context::work work(io_context);

    // Run the io_context event loop
    io_context.run();

    std::cout << "printing line" << std::endl;
    return 0;
}

void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, uint8_t* data, udp::endpoint& sender_endpoint, boost::asio::io_context& io_context) {
    if (!error) {
        // Convert the received header data to uint32_t
        uint32_t header = *reinterpret_cast<uint32_t*>(data);
        std::cout << "Header: " << header << std::endl;

        if (header >= 1111111111) { // config packet
            // Read 400 bytes
//            char data_buffer[400];
//            socket.async_receive_from(boost::asio::buffer(data_buffer, 400), sender_endpoint,
//                                      [&socket, &data_buffer, &sender_endpoint, &io_context](const boost::system::error_code& error, std::size_t bytes_transferred) {
//                                          handle_receive_data(error, bytes_transferred, socket, data_buffer, sender_endpoint, io_context);
//                                      });
            std::cout << "config packet" << error.message() << std::endl;

        } else { // regular packet
            // Read 10000 bytes
//            char data_buffer[max_length];
//            socket.async_receive_from(boost::asio::buffer(data_buffer, max_length), sender_endpoint,
//                                      [&socket, &data_buffer, &sender_endpoint, &io_context](const boost::system::error_code& error, std::size_t bytes_transferred) {
//                                          handle_receive_data(error, bytes_transferred, socket, data_buffer, sender_endpoint, io_context);
//                                      });
            std::cout << "regular packet" << error.message() << std::endl;
        }
        // else {
//            std::cerr << "Unknown header: " << header << std::endl;
//        }
    } else {
        std::cerr << "Error receiving header: " << error.message() << std::endl;
    }
}

//void handle_receive_data(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, char* data, udp::endpoint& sender_endpoint, boost::asio::io_context& io_context) {
//    if (!error) {
//        // Process received data based on the header and sender endpoint
//        // Example: Print the received data along with sender information
//        std::cout << "Received " << bytes_transferred << " bytes from " << sender_endpoint << ": " << std::string(data+4, bytes_transferred) << std::endl;
//
//        // Initiate another async receive operation
//        char header_data[header_length];
//        socket.async_receive_from(boost::asio::buffer(header_data, header_length), sender_endpoint,
//                                  [&socket, &header_data, &sender_endpoint, &io_context](const boost::system::error_code& error, std::size_t bytes_transferred) {
//                                      handle_receive_header(error, bytes_transferred, socket, header_data, sender_endpoint, io_context);
//                                  });
//    } else {
//        std::cerr << "Error receiving data: " << error.message() << std::endl;
//    }
//}
