#include "serverHandling.hpp"

void serverHandling::receiveConfigPacket(const boost::system::error_code& error, std::size_t bytes_transferred,
                                                             boost::asio::ip::udp::endpoint sender_endpoint,
                                                             boost::asio::ip::udp::socket& socket,
                                                             boost::asio::io_context& io_context) {

    std::vector<uint8_t> receive_buffer(1);
    std::size_t bytesRead = socket.async_receive_from(boost::asio::buffer(receive_buffer), sender_endpoint, handle_receive);

    io_context.run();
}

void serverHandling::receivePacket(boost::asio::ip::udp::socket& socket, unsigned long packetsNumber,
                                   unsigned int chunkSize, std::string& finalPath) {
    std::vector<std::string> packetsVector(packetsNumber);
    FILE_STORAGE::Packet packet; // creating a packet object
    char receive_data[chunkSize + 8]; // getting the data from socket
    boost::asio::ip::udp::endpoint client;
    boost::system::error_code error;
    for (unsigned long i = 0; i < packetsNumber; ++i)
    {
        std::size_t bytesRead = socket.receive_from(boost::asio::buffer(receive_data, sizeof(receive_data)), client, 0, error);
        if (error)
            std::cerr << "Error receiving data: " << error.message() << std::endl;
        std::cout << "Received " << bytesRead << " bytes from the client." << std::endl;
        std::cout << "NEW PACKET FROM CLIENT CONNECTION: " << client << std::endl;
        packet.ParseFromString(std::string(receive_data, bytesRead)); // parse back all the bytes that read
        std::cout << "PACKET RECEIVE SIZE: " << packet.ByteSizeLong() << "." << std::endl;
        std::cout << "PACKET FILE CONTENT RECEIVE SIDE: \n" << packet.file_content() << "." << std::endl;
        packetsVector[packet.id() - 1] = packet.file_content();
        memset(receive_data, 0, sizeof(receive_data));
    }
    createFileAndWrite(packetsVector, packetsNumber, finalPath);
}

void serverHandling::createFileAndWrite(std::vector<std::string>& contentVector, unsigned long packetsNumber, std::string& finalPath)
{
    std::string finalFileContent;
    std::ofstream file(finalPath); // opening the file in writing mode
    unsigned int lostPackets = 0;
    for (unsigned long i = 0; i < packetsNumber; ++i)
    {
        if (contentVector[i].empty())
        {
            std::cout << "Packet number: " << i+1 << " get lost..." << std::endl;
            lostPackets += 1;
        }
            // delete the nulls at the end from the first/last packet
        else if (i == packetsNumber - 1)
            contentVector[i].erase(std::find(contentVector[i].begin(),contentVector[i].end(), '\0'),
                                   contentVector[i].end());

        finalFileContent += contentVector[i];
        if (finalFileContent.length() > 512)
        {
            std::cout << "get some space in the vector..." << std::endl;
            file << finalFileContent;
            finalFileContent = "";
        }
    }
    file << finalFileContent;
    file.flush(); // flushing the output file stream // does not working well...

    file.close();
    std::cout << "The number of lost packets is: " << lostPackets << " packets." << std::endl;
}