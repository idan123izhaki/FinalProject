#include "ClientSession.hpp"

ClientSession::ClientSession(int session_number, std::string destinationIP, unsigned short destinationPort, boost::asio::io_context& io_context_)
        : io_context(io_context_),
        remote_endpoint(boost::asio::ip::udp::endpoint(boost::asio::ip::make_address(destinationIP), destinationPort)),
        socket(io_context)
{
    std::cout << "Hello from the ClientSession class!" << std::endl;
    socket.open(boost::asio::ip::udp::v4());
    this->destinationIP = destinationIP;
    this->destinationPort = destinationPort;
    this->session_number = session_number;
    std::cout << "-> client session number: " << this->session_number << " created successfully." << std::endl;;
}


void ClientSession::sendingPackets(std::vector<uint8_t> packet) {
    this->socket.async_send_to(boost::asio::buffer(packet), this->remote_endpoint,
                               [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                                            if ((!ec || ec == boost::asio::error::message_size) && bytes_transferred > 0)
                                                std::cout << "Sent " << bytes_transferred << " bytes, to- IP = " <<
                                                    this->remote_endpoint.address().to_string() << ", PORT = " << this->remote_endpoint.port() << std::endl;
                                            else
                                                std::cerr << "Error sending packet: " << ec.message() << std::endl;
                               });
}

//void ClientSession::startSending() {
//    io_context.run();
//}

int ClientSession::getSessionNumber() const{
    return this->session_number;
}


ClientSession::~ClientSession() {
    this->socket.close(); // only while the session removed by the user or something
    std::cout << "client session number: " << this->session_number << " closed." << std::endl;
}
