#include "FileManagement.hpp"

//namespace RaptorQ = RaptorQ__v1;
//// RaptorQ configuration
//constexpr uint16_t symbolSize = 100;  // 100 bytes per symbol
//constexpr uint16_t blockSize = 4;     // Choose an appropriate block size

#define REPEAT 5 // sending the config packet

// opening a UDP socket and configuring it to use IPv4 addressing
//FileManagement::FileManagement(boost::asio::io_context& io_context, unsigned short port): socket(io_context)
//{
//    // specify the IP version to use
//    socket.open(boost::asio::ip::udp::v4());
//    this->port = port;
//}

// boost::asio::ip::udp::socket& UdpClient::getSocket() {
//     return this->socket;
// }

// checking the user path
std::string FileManagement::pathHandler()
{
    std::string path;
    std::cout << "Please enter a path of a file/directory: " << std::endl;
    bool flag = false;
    while(!flag)
    {
        std::cin >> path;
        // check if the current path is exist
        if (std::filesystem::exists(path)) {
            if (std::filesystem::is_directory(path)) {
                std::cout << "This is a directory!" << std::endl;
            }
            else if (std::filesystem::is_regular_file(path)) {
                std::cout << "This is a file!" << std::endl;
            }
            flag = true;
        }
        else {
            std::cout << "Try again..." << std::endl;
            std::cout << "Enter a valid path name: " << std::endl;
        }
    }
    return path;
}

// create and send the first packet -> configuration packet
void FileManagement::createAndSendConfigPacket(const std::string& ipAddress, std::string& path, std::string& currentName,
                                          unsigned long chunkSize, uint8_t conType, uint32_t symbol_size, uint32_t overhead) {
    FILE_STORAGE::ConfigPacket configPacket;
    if (std::filesystem::is_directory(path))
        configPacket.set_type(FILE_STORAGE::FileType::DIRECTORY);
    else
    {
        configPacket.set_type(FILE_STORAGE::FileType::FILE);
        configPacket.set_con_type((!conType) ? FILE_STORAGE::ContentType::TEXT : FILE_STORAGE::ContentType::BINARY);
        std::uint64_t fileSize = std::filesystem::file_size(path);
        configPacket.set_chunks((fileSize % chunkSize) ? (fileSize/chunkSize) + 1 : fileSize/chunkSize);
        configPacket.set_chunk_size(chunkSize);
        configPacket.set_block_size(static_cast<uint32_t>(fec::getBlockSize(fec::getSymbolNum(
                chunkSize, symbol_size))));
        configPacket.set_symbol_size(symbol_size);
        configPacket.set_overhead(overhead);
    }
    configPacket.set_name(currentName);

    uint8_t typePacket = 1; // because this is a configuration packet
    uint32_t configID = 0;
    std::string serialized_data = configPacket.SerializeAsString();

    std::vector<uint8_t> finalData;

    // Copy the bytes of num1 and num2 into the vector
    finalData.resize(sizeof(uint8_t));
    std::memcpy(finalData.data(), &typePacket, sizeof(uint8_t));
    std::memcpy(finalData.data() + sizeof(uint8_t), &configID, sizeof(uint32_t));
    // insert the data info into the vector
    finalData.insert(finalData.end(), serialized_data.begin(), serialized_data.end());

    // the finalData: [4byte-typePacket | 4bytes-configID | serialized data]

    // sending the configuration packet (sending it REPEAT times -> repetition functionality
    for (int i = 0; i < REPEAT; ++i) {
        mutex_socket.lock();
        this->socket.send_to(boost::asio::buffer(serialized_data),
                             boost::asio::ip::udp::endpoint{boost::asio::ip::make_address(ipAddress), this->port});
        mutex_socket.unlock();
    }
}

// doing serialization to file in the path and send it in multiple chunks
void FileManagement::fileSender(const std::string& ipAddress, std::string& path,
                           unsigned long chunkSize, uint32_t symbol_size, uint32_t overhead)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Error while opening the file..." << std::endl;
        return;
    }
//    std::string chunkInfo;
    std::vector<char> buffer(chunkSize);
    uint32_t chunkId = 0;
    std::vector<std::pair<uint32_t, std::vector<uint8_t>>> received_symbols;
    while(!file.eof())
    {
        file.read(buffer.data(), chunkSize);
        //packet.set_id(id);
        //packet.set_file_content(std::string(buffer.begin(), buffer.end()));
        //std::cout << "Packet ID:  " << packet.id() << std::endl;
        //std::cout << "Packet file content in chunk of " << chunkSize << " bytes is: \n" << packet.file_content() << std::endl;

        // adding this lines - updating the code with using now the libRaptorQ
        //std::string serialized_data = packet.SerializeAsString();

        //     // encode static function
        //    static std::vector<std::pair<uint32_t, std::vector<uint8_t>>> encoder(
        //            const std::string& inputString, uint32_t symbol_size, const uint8_t overhead);
        received_symbols = fec::encoder(std::string(buffer.begin(), buffer.end()),
                                        symbol_size, overhead);
        std::cout << "***********************************************************************" << std::endl << std::endl;
        std::cout << "number of symbols after encoding: " << received_symbols.size() << std::endl;

//        std::vector<std::pair<uint32_t, std::vector<uint8_t>>>
//                sliced_vec(received_symbols.begin() + 7, received_symbols.end());

        mutex_socket.lock();
        for (auto &encoded_symbol : received_symbols)
        {
            std::size_t bytesSent = this->socket.send_to(
                    boost::asio::buffer(fec::createVector(chunkId, encoded_symbol.first, encoded_symbol.second)),
                    boost::asio::ip::udp::endpoint{boost::asio::ip::make_address(ipAddress), this->port});
            std::cout << "BYTES THAT SENT IN THE " << chunkId << "---" << encoded_symbol.first << " PACKET ARE: "
            << bytesSent << " BYTES." << std::endl;
            std::cout << "id symbol: " << encoded_symbol.first << ", encoded data: " <<
            std::string(encoded_symbol.second.begin(), encoded_symbol.second.end())<< std::endl;
        }
        mutex_socket.unlock();

//        mutex_socket.lock();
//        std::size_t bytesSent = this->socket.send_to(boost::asio::buffer(serialized_data),
//                       boost::asio::ip::udp::endpoint{boost::asio::ip::make_address(ipAddress), this->port});
//        mutex_socket.unlock();
//        std::cout << "BYTES THAT SENT IN THE " << id << " PACKET ARE: " << bytesSent << " BYTES." << std::endl;
//        ++id;

        // reset vector elements
        received_symbols.clear(); // encoded vector
        buffer.assign(buffer.size(), 0); // file buffer
        ++chunkId;
    }
    file.close();
}

void FileManagement::directory_file_scanner(std::string path, unsigned long chunkSize, uint32_t symbol_size,
                                       uint32_t overhead, std::string baseName, int inotify_fd)
{
    if (std::filesystem::is_directory(path)) {
        std::string currentDir =  baseName + std::filesystem::path(path).filename().string(); // directory name
        int watch_fd = addPathToMonitor(inotify_fd, path);
        mutex_structure.lock();
        this->pathsMap[watch_fd] = std::make_pair(path, currentDir);
        mutex_structure.unlock();
        this->createAndSendConfigPacket("127.0.0.1", path,currentDir, chunkSize,
                                        0, symbol_size, overhead);
        for (const auto &entry: std::filesystem::directory_iterator(path)) {
            if (std::filesystem::is_regular_file(entry))
            {
                std::string currentPath = entry.path().string();
                std::string currentFileName = currentDir + '/' + entry.path().filename().string();
                this->createAndSendConfigPacket("127.0.0.1", currentPath, currentFileName, chunkSize,
                                                 0, symbol_size, overhead); // configPacket for each file
                this->fileSender("127.0.0.1", currentPath, chunkSize, symbol_size, overhead);
            }
            else // if directory
            {
                std::string base = currentDir + '/' ;
                std::string newPath = entry.path().string();
                directory_file_scanner(newPath, chunkSize, symbol_size, overhead, base, inotify_fd);
            }
        }
    }
    else // sending a single file (not a directory)
    {
        std::string fileName = std::filesystem::path(path).filename().string(); // directory name
        int watch_fd = addPathToMonitor(inotify_fd, path);
        mutex_structure.lock();
        this->pathsMap[watch_fd] = std::make_pair(path, fileName);
        mutex_structure.unlock();
        this->createAndSendConfigPacket("127.0.0.1", path, fileName, chunkSize,
                                        0, symbol_size, overhead); // configPacket first
        fileSender("127.0.0.1", path, chunkSize, symbol_size, overhead);
        std::cout << "SENDING DATA DONE." << std::endl;
    }
}

int FileManagement::init_inotify_obj()
{
    int inotify_fd = inotify_init();
    if (inotify_fd == -1)
    {
        perror("inotify_init");
        return -1;
    }
    return inotify_fd;
}

int FileManagement::addPathToMonitor(int inotify_fd, std::string& path)
{
    if (std::filesystem::exists(path))
    {
        int new_watch;
        if (std::filesystem::is_directory(path))
            new_watch = inotify_add_watch(inotify_fd, path.c_str(), IN_CREATE | IN_MODIFY);
        else if (std::filesystem::is_regular_file(path))
            new_watch = inotify_add_watch(inotify_fd, path.c_str(), IN_MODIFY);

        if (new_watch == -1)
        {
            perror("inotify_add_watch");
            return -1;
        }
        return new_watch;
    }
    else
        return -1;
}

// adding monitor to current directory
void FileManagement::monitorFunc(int inotify_fd, unsigned long chunkSize, uint32_t symbol_size, uint32_t overhead)
{
    std::cout << "IN THE MONITORING THREAD!!" << std::endl;
    std::cout << "Map Contents:" << std::endl;
    for (const auto& pair : this->pathsMap) {
        std::cout << pair.first << ": " << pair.second.first << ", " << pair.second.second << std::endl;
    }
    if (inotify_fd != -1)
    {
        std::string currentPath, currentName;
        while(true)
        {
            char buffer[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
            ssize_t length = read(inotify_fd, buffer, sizeof(buffer));
            if (length == -1)
            {
                perror("read failed");
                break;
            }
            const struct inotify_event* event = reinterpret_cast<const struct inotify_event*>(buffer);
            if (std::string(event->name).find(".swp") == std::string::npos && std::string(event->name).find(".goutputstream") == std::string::npos)
            {
                if (event->mask & IN_CREATE || event->mask & IN_MODIFY) {
                    if (event->mask & IN_MODIFY)
                        std::cout << "File modified: " << event->name << std::endl;
                    else
                        std::cout << "File created: " << event->name << std::endl;

                    std::string firstArg, secondArg;
                    firstArg = this->pathsMap[event->wd].first; // the parent path
                    secondArg = this->pathsMap[event->wd].second; // the base name

                    std::cout << "watch descriptor: " << event->wd << std::endl;
                    std::cout << "Parent path: " << firstArg << std::endl;
                    if(std::filesystem::is_directory(firstArg)) {
                        currentPath = this->pathsMap[event->wd].first + '/' + event->name; // the current path of the new file/dir
                        currentName = this->pathsMap[event->wd].second + '/' + event->name; // base name (if exist)
                    }
                    else {
                        currentPath = firstArg; // the current path of the new file/dir
                        currentName = secondArg;
                    }

                    std::cout << "currentPath: " << currentPath << std::endl;
                    std::cout << "currentName: " << currentName << std::endl;

                    this->createAndSendConfigPacket("127.0.0.1", currentPath, currentName, chunkSize,
                                                    0, symbol_size, overhead);
                    if (std::filesystem::is_regular_file(currentPath))
                        this->fileSender("127.0.0.1", currentPath, chunkSize, symbol_size, overhead); // send the file content
                    else
                    {
                        // adding the new directory into the data structure
                        int watch_fd = addPathToMonitor(inotify_fd, currentPath);
                        mutex_structure.lock();
                        this->pathsMap[watch_fd] = std::make_pair(currentPath, currentName);
                        mutex_structure.unlock();
                    }
                }
            }
        }
    }
    else
    {
        std::cerr << "Failed while init inotify..." << std::endl;
    }
}
