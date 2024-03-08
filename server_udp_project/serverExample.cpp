////#include <boost/asio.hpp>
////#include <iostream>
////
////using boost::asio::ip::udp;
////
////const int header_length = 4;
////const int max_length = 10000;
////
////void handle_receive_header(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, char* data, udp::endpoint& sender_endpoint);
////
////void handle_receive_data(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, char* data, udp::endpoint& sender_endpoint);
////
////int main() {
////    boost::asio::io_context io_context;
////    udp::socket socket(io_context, udp::endpoint(udp::v4(), 1234)); // UDP socket bound to port 1234
////
////    char header_data[header_length];
////    udp::endpoint sender_endpoint;
////    socket.async_receive_from(boost::asio::buffer(header_data, header_length), sender_endpoint,
////                              [&socket, &header_data, &sender_endpoint](const boost::system::error_code& error, std::size_t bytes_transferred) {
////                                  handle_receive_header(error, bytes_transferred, socket, header_data, sender_endpoint);
////                              });
////
////    io_context.run();
////    std::cout << "printing line!" << std::endl;
////    return 0;
////}
////
////void handle_receive_header(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, char* data, udp::endpoint& sender_endpoint) {
////    if (!error) {
////        // Convert the received header data to uint32_t
////        uint32_t header = *reinterpret_cast<uint32_t*>(data);
////
////        if (header > 1111111111) {
////            // Read 400 bytes
////            char data_buffer[400];
////            socket.async_receive_from(boost::asio::buffer(data_buffer, 400), sender_endpoint,
////                                      [&socket, &data_buffer, &sender_endpoint](const boost::system::error_code& error, std::size_t bytes_transferred) {
////                                          handle_receive_data(error, bytes_transferred, socket, data_buffer, sender_endpoint);
////                                      });
////        } else if (header < 1111111111) {
////            // Read 10000 bytes
////            char data_buffer[max_length];
////            socket.async_receive_from(boost::asio::buffer(data_buffer, max_length), sender_endpoint,
////                                      [&socket, &data_buffer, &sender_endpoint](const boost::system::error_code& error, std::size_t bytes_transferred) {
////                                          handle_receive_data(error, bytes_transferred, socket, data_buffer, sender_endpoint);
////                                      });
////        } else {
////            std::cerr << "Unknown header: " << header << std::endl;
////        }
////    } else {
////        std::cerr << "Error receiving header: " << error.message() << std::endl;
////    }
////}
////
////void handle_receive_data(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, char* data, udp::endpoint& sender_endpoint) {
////    if (!error) {
////        // Process received data based on the header and sender endpoint
////        // Example: Print the received data along with sender information
////        std::cout << "Received " << bytes_transferred << " bytes from " << sender_endpoint << ": " << std::string(data+4, bytes_transferred) << std::endl;
////    } else {
////        std::cerr << "Error receiving data: " << error.message() << std::endl;
////    }
////}
//
//
//
//#include <boost/asio.hpp>
//#include <iostream>
//
//using boost::asio::ip::udp;
//
//const int max_packet_length = 10000; // max_size_length for any-data
//
//enum Header
//{   config = 0,
//    regular = 1
//};
//
//void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, uint8_t* data, udp::endpoint& sender_endpoint, boost::asio::io_context& io_context);
//
////void handle_receive_data(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, char* data, udp::endpoint& sender_endpoint, boost::asio::io_context& io_context);
//
//int main() {
//    boost::asio::io_context io_context;
//    udp::socket socket(io_context, udp::endpoint(udp::v4(), 1234)); // UDP socket bound to port 1234
//
//    uint8_t packet[max_packet_length];
//    udp::endpoint sender_endpoint;
//
//    // Start the initial async receive operation
//    socket.async_receive_from(boost::asio::buffer(packet, max_packet_length), sender_endpoint,
//                              [&socket, &packet, &sender_endpoint, &io_context](const boost::system::error_code& error, std::size_t bytes_transferred) {
//                                  handle_receive(error, bytes_transferred, socket, packet, sender_endpoint, io_context);
//                              });
//
//    // Create a work object to keep the io_context alive
//    //boost::asio::io_context::work work(io_context);
//
//    // Run the io_context event loop
//    io_context.run();
//
//    std::cout << "printing line" << std::endl;
//    return 0;
//}
//
//void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, uint8_t* data, udp::endpoint& sender_endpoint, boost::asio::io_context& io_context) {
//    if (!error) {
//        // Convert the received header data to uint32_t
//        uint32_t header = *reinterpret_cast<uint32_t*>(data);
//        std::cout << "Header: " << header << std::endl;
//
//        if (header >= 1111111111) { // config packet
//            // Read 400 bytes
////            char data_buffer[400];
////            socket.async_receive_from(boost::asio::buffer(data_buffer, 400), sender_endpoint,
////                                      [&socket, &data_buffer, &sender_endpoint, &io_context](const boost::system::error_code& error, std::size_t bytes_transferred) {
////                                          handle_receive_data(error, bytes_transferred, socket, data_buffer, sender_endpoint, io_context);
////                                      });
//            std::cout << "config packet" << error.message() << std::endl;
//
//        } else { // regular packet
//            // Read 10000 bytes
////            char data_buffer[max_length];
////            socket.async_receive_from(boost::asio::buffer(data_buffer, max_length), sender_endpoint,
////                                      [&socket, &data_buffer, &sender_endpoint, &io_context](const boost::system::error_code& error, std::size_t bytes_transferred) {
////                                          handle_receive_data(error, bytes_transferred, socket, data_buffer, sender_endpoint, io_context);
////                                      });
//            std::cout << "regular packet" << error.message() << std::endl;
//        }
//        // else {
////            std::cerr << "Unknown header: " << header << std::endl;
////        }
//    } else {
//        std::cerr << "Error receiving header: " << error.message() << std::endl;
//    }
//}
//
////void handle_receive_data(const boost::system::error_code& error, std::size_t bytes_transferred, udp::socket& socket, char* data, udp::endpoint& sender_endpoint, boost::asio::io_context& io_context) {
////    if (!error) {
////        // Process received data based on the header and sender endpoint
////        // Example: Print the received data along with sender information
////        std::cout << "Received " << bytes_transferred << " bytes from " << sender_endpoint << ": " << std::string(data+4, bytes_transferred) << std::endl;
////
////        // Initiate another async receive operation
////        char header_data[header_length];
////        socket.async_receive_from(boost::asio::buffer(header_data, header_length), sender_endpoint,
////                                  [&socket, &header_data, &sender_endpoint, &io_context](const boost::system::error_code& error, std::size_t bytes_transferred) {
////                                      handle_receive_header(error, bytes_transferred, socket, header_data, sender_endpoint, io_context);
////                                  });
////    } else {
////        std::cerr << "Error receiving data: " << error.message() << std::endl;
////    }
////}







//#include <boost/asio.hpp>
//#include <iostream>
//#include <vector>
//
//using boost::asio::ip::udp;
//
//class UDPServer {
//public:
//    UDPServer(boost::asio::io_context& io_context, const std::string& listen_address, const std::string& listen_port)
//            : socket_(io_context, udp::endpoint(boost::asio::ip::address::from_string(listen_address), std::stoi(listen_port))),
//              recv_buffer_(1024){
//        startReceive();
//    }
//
//private:
//    void startReceive() {
//        socket_.async_receive_from(
//                boost::asio::buffer(recv_buffer_), remote_endpoint_,
//                [this](boost::system::error_code ec, std::size_t bytes_received) {
//                    if (!ec) {
//                        std::cout << "Received " << bytes_received << " bytes from "
//                                  << remote_endpoint_.address().to_string() << ":" << remote_endpoint_.port() << std::endl;
//
//                        // Process received data here
//                        std::cout << "Received data: " << std::string(recv_buffer_.begin(), recv_buffer_.begin() + bytes_received) << std::endl;
//
//                        // Continue to receive
//                        startReceive();
//                    } else {
//                        std::cerr << "Receive error: " << ec.message() << std::endl;
//                    }
//                });
//    }
//
//    udp::socket socket_;
//    udp::endpoint remote_endpoint_;
//    std::vector<uint8_t > recv_buffer_;// Adjust buffer size as needed
//
//};
//
//int main() {
//    boost::asio::io_context io_context;
//    std::string listen_address = "0.0.0.0"; // Listen on all available network interfaces
//    std::string listen_port = "1234";       // Port to listen on
//
//    UDPServer server(io_context, listen_address, listen_port);
//
//    io_context.run(); // Run the io_context event loop
//
//    return 0;
//}


//#include <iostream>
//#include <map>
//
//class FileBuilder {
//public:
//    FileBuilder(int value) {
//        std::cout << "FileBuilder constructor called" << std::endl;
//        this->value = value;
//    }
//
//    int getValue() const
//    {
//        return this->value;
//    }
//    ~FileBuilder() {
//        std::cout << "FileBuilder destructor called- value: " << this->value << std::endl;
//    }
//private:
//    int value;
//};
//
//int main() {
//    std::map<uint32_t, std::shared_ptr<FileBuilder>> my_map;
//
//    my_map.erase(1);
//    for (const auto& pair : my_map) {
//        std::cout << "Key: " << pair.first << ", Value: " << pair.second.getValue() << std::endl;
//    }
//    std::cout << "END OF PROGRAM" << std::endl;
//    std::cout << "Key: " << 1 << ", Value: " << my_map.at(2).getValue() << std::endl;
//
//    return 0;
//} // FileBuilder destructor will be called here, when my_map goes out of scope

//#include <iostream>
//#include <vector>
//#include <memory>
//
//// Sample class
//class MyClass {
//public:
//    MyClass(int value1, int value2) : data1(value1), data2(value2) {
//        std::cout << "MyClass constructor with value: " << data1 << "," << data2 << std::endl;
//    }
//
//    ~MyClass() {
//        std::cout << "MyClass destructor with value: " << data1 << "," << data2 << std::endl;
//    }
//
//    void doSomething() const {
//        std::cout << "Doing something with value: " << data1 << "," << data2 << std::endl;
//    }
//
//private:
//    int data1, data2;
//};
//
//int main() {
//    // Vector of pairs with unique_ptr
//    std::vector<std::pair<int, std::unique_ptr<MyClass>>> vec;
//
//    // Adding items to the vector
//    vec.emplace_back(1, std::make_unique<MyClass>(10, 20));
//    vec.emplace_back(2, std::make_unique<MyClass>(100, 200));
//
//    // Accessing elements and calling member functions
//    for (const auto& item : vec) {
//        std::cout << "Element " << item.first << ": ";
//        item.second->doSomething();
//    }
//
//    vec.erase(vec.begin());
//    std::cout <<"hello world, after deleting the first item with value 10, 20." << std::endl;
//
//    // Memory is automatically released when vector goes out of scope
//    return 0;
//}

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

class MyClass {
public:
    MyClass() : running(true), thread(&MyClass::workerThread, this) {}

    ~MyClass() {
        running = false; // Set flag to signal thread termination
        if (thread.joinable()) {
            thread.join(); // Wait for the thread to finish
        }
    }

private:
    std::atomic<bool> running;
    std::thread thread;

    void workerThread() {
        while (running) {
            // Do some work in the thread
            std::cout << "Thread is running..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(4));
        }
        std::cout << "Thread stopped." << std::endl;
    }
};

int main() {
    {
        MyClass obj; // Create an instance of MyClass
        std::this_thread::sleep_for(std::chrono::seconds(6)); // Let the thread run for 5 seconds
    } // Destructor of MyClass called here
    std::cout << "object destroyed, out of scope..." << std::endl;
    return 0;
}
