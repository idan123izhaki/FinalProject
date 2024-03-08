#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <filesystem>
#include <atomic>
#include <thread>
#include <boost/asio.hpp>
#include "fecAlgorithm.hpp"
#include "../fileStructure.pb.h" // protocol buffer

#ifndef SERVER_UDP_PROJECT_FILEBUILDER_HPP
#define SERVER_UDP_PROJECT_FILEBUILDER_HPP


class FileBuilder {
    uint32_t file_id;
    std::string path;
    bool mode; // text and binary types - true if text
    uint64_t chunks_number, received_packets;
    uint32_t chunk_size, symbol_size, overhead, symbols_number;
    std::map<uint32_t, std::vector<std::pair<uint32_t, std::vector<uint8_t>>>> chunks_symbols_map; //key: chunk_id, value: vector symbols
    std::vector<std::vector<uint8_t>> decoded_info; //each place representing a final chunk (all the symbols of chunk after decoding).
    std::atomic<bool> runningFlag;
    std::thread thread;
    std::mutex decoded_info_mutex;


public:
    //constructor
    FileBuilder(uint32_t file_id, std::string path, bool mode, uint64_t chunks_number, uint32_t symbols_number,
                uint32_t chunk_size, uint32_t symbol_size, uint32_t overhead);

    void add_symbol(uint32_t chunk_id, std::pair<uint32_t,std::vector<uint8_t>>& symbol_raw);
    void add_decode_data(uint32_t chunk_id, std::vector<uint8_t>& decoded_data);
    void writeToTextFile();
    void writeToBinaryFile();
    uint32_t gettingLostPacketsNum() const;
    void writeEachSecond();
    void writingBeforeClosing();
    //static void sessionHandling(unsigned short port);
    //static void receiveHandling(const boost::system::error_code& error, std::size_t bytes_transferred);

    //destructor
    ~FileBuilder();
};

#endif //SERVER_UDP_PROJECT_FILEBUILDER_HPP
