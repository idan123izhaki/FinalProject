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
#include <memory>
#include "../fileStructure.pb.h" // protoBuf file
#include "fecAlgorithm.hpp" // fec algorithm file
#include "ClientSession.hpp" // manages the socket session

#ifndef CLIENT_UDP_PROJECT_FILEHANDLER_HPP
#define CLIENT_UDP_PROJECT_FILEHANDLER_HPP

class FileManagement {
    std::unique_ptr<ClientSession> session;
    static uint32_t fileId;
    std::string path;
    uint32_t chunk_size, symbol_size, overhead;
    int inotify_fd;
    std::mutex mutex_structure; // see word document, point 3
    std::map<int, std::pair<std::string, std::string>> map_path; // for listening changes
public:

    FileManagement(std::unique_ptr<ClientSession> session, std::string path, uint32_t chunk_size, uint32_t symbol_size, uint32_t overhead);

    static std::string pathHandler();

    bool isTextFile(const std::string& path);

    std::vector<uint8_t> createHeader(bool isRegular, uint32_t conReg, uint32_t fileId, uint64_t chunk_id=0, uint32_t symbol_id=0);

    //uint32_t getNextFileId();

    void createAndSendConfigPacket(uint32_t fileId, std::string& path, std::string& currentName);

    void fileSender(uint32_t fileId, std::string& path);

    void directory_file_scanner(std::string path, std::string baseName);

    int init_inotify_obj();

    int addPathToMonitor(int inotify_id, std::string& path);

    void monitorFunc(int inotify_fd, uint32_t chunkSize, uint32_t symbol_size, uint32_t overhead);

    void startSending();

    ~FileManagement();
};

#endif //CLIENT_UDP_PROJECT_FILEHANDLER_HPP
