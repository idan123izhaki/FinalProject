#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include <boost/asio.hpp>
#include <sys/inotify.h>
#include <mutex>
#include <thread>
#include <map>
#include <vector>
#include "../fileStructure.pb.h" // protoBuf file
#include "fecAlgorithm.hpp" // fec algorithm file
//#include "ClientSession.hpp" // manages the session

#ifndef CLIENT_UDP_PROJECT_FILEHANDLER_HPP
#define CLIENT_UDP_PROJECT_FILEHANDLER_HPP

class FileManagement {
    //ClientSession session;
    uint32_t fileId;
public:

    //FileManagement(boost::asio::io_context& io_context, unsigned short port);

    static std::string pathHandler();

    std::vector<uint8_t> createHeader(bool isRegular, uint32_t conReg, uint32_t fileId, uint64_t chunk_id=0, uint32_t symbol_id=0);

    void createAndSendConfigPacket(const std::string& ipAddress, std::string& path, std::string& currentName,
                                   unsigned long chunkSize, uint8_t conType, uint32_t symbol_size, uint32_t overhead);

    void fileSender(const std::string& ipAddress, std::string& path,
                    unsigned long chunkSize, uint32_t symbol_size, uint32_t overhead);

    void directory_file_scanner(std::string path, unsigned long chunkSize, uint32_t symbol_size, uint32_t overhead,
                                std::string baseName, int inotify_fd);

    int init_inotify_obj();

    int addPathToMonitor(int inotify_id, std::string& path);

    void monitorFunc(int inotify_fd, unsigned long chunkSize, uint32_t symbol_size, uint32_t overhead);
};

#endif //CLIENT_UDP_PROJECT_FILEHANDLER_HPP
