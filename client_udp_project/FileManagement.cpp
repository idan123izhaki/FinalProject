#include "FileManagement.hpp"

//namespace RaptorQ = RaptorQ__v1;
//// RaptorQ configuration
//constexpr uint16_t symbolSize = 100;  // 100 bytes per symbol
//constexpr uint16_t blockSize = 4;     // Choose an appropriate block size

#define REPEAT 5 // sending the config packet

uint32_t FileManagement::fileId = 0;

FileManagement::FileManagement(std::unique_ptr<ClientSession> session_,
                               std::string path, uint32_t chunk_size, uint32_t symbol_size, uint32_t overhead)
                               : session(std::move(session_))
{
    this->path = path;
    this->chunk_size = chunk_size;
    this->symbol_size = symbol_size;
    this->overhead = overhead;
    this->inotify_fd = init_inotify_obj();
    std::cout << "-> FileManagement created successfully, calling the 'directory_file_scanner' function..." << std::endl;
    directory_file_scanner(path, ""); // sending all to server side
}


//uint32_t FileManagement::getNextFileId() {
//    uint32_t currentId = FileManagement::fileId;
//    ++FileManagement::fileId;
//    return currentId;
//}

// checking the user path
std::string FileManagement::pathHandler()
{
    std::string path;
    std::cout << "Please enter a valid path of file/directory: " << std::endl;
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
        else
            std::cout << "Try again... Enter a valid path name: " << std::endl;
    }
    return path;
}

bool FileManagement::isTextFile(const std::string& path) {
    std::string command = "file " + path;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "Error executing command.";
    }

    char buffer[128];
    std::string result;
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    std::cout << result << std::endl;
    return result.find("text") != std::string::npos;
}


std::vector<uint8_t> FileManagement::createHeader(bool isRegular, uint32_t conReg, uint32_t fileId, uint64_t chunk_id, uint32_t symbol_id) {
    std::vector<uint8_t> header;

    header.resize(sizeof(uint32_t) * 2);

    std::memcpy(header.data(), &conReg, sizeof(uint32_t)); // adding the packet type- config ot regular
    std::memcpy(header.data() + sizeof(uint32_t), &fileId, sizeof(uint32_t));
    if (isRegular)
    {
        header.resize(sizeof(uint32_t) * 5);
        std::memcpy(header.data() + sizeof(uint64_t) , &chunk_id, sizeof(uint64_t)); // adding the packet type - config or regular
        std::memcpy(header.data() + sizeof(uint64_t) * 2, &symbol_id, sizeof(uint32_t));
    }
    std::cout <<" --- THE HEADER SIZE IS: " << header.size() << " BYTES." << std::endl;
    return header;
}


// create and send the configuration packet
void FileManagement::createAndSendConfigPacket(uint32_t fileId, std::string& path, std::string& currentName) {
    FILE_STORAGE::ConfigPacket configPacket;
    if (std::filesystem::is_directory(path))
        configPacket.set_type(FILE_STORAGE::FileType::DIRECTORY);
    else
    {
        configPacket.set_type(FILE_STORAGE::FileType::FILE);
        configPacket.set_con_type(isTextFile(path) ? FILE_STORAGE::ContentType::TEXT : FILE_STORAGE::ContentType::BINARY);
        std::uint64_t fileSize = std::filesystem::file_size(path);
        configPacket.set_chunks((fileSize % this->chunk_size) ? (fileSize/this->chunk_size) + 1 : fileSize/this->chunk_size);
        configPacket.set_chunk_size(this->chunk_size);
        configPacket.set_block_size(static_cast<uint32_t>(fec::getBlockSize(fec::getSymbolNum(
                this->chunk_size, this->symbol_size))));
        configPacket.set_symbol_size(this->symbol_size);
        configPacket.set_overhead(this->overhead);
    }
    // current name is equal to the file name with the previous directories names if exist (for the server to know where to place the file)
    configPacket.set_name(currentName);

    uint32_t typePacket = 100; // because this is a configuration packet- 200 if regular packet
    std::string serialized_data = configPacket.SerializeAsString();

    // the finalConfigPacket: [4byte-typePacket | 4bytes-configID | serialized data]
    std::vector<uint8_t> finalConfigPacket = createHeader(false, typePacket, fileId); // after this the packet is ready with relevant headers
    finalConfigPacket.insert(finalConfigPacket.end(), serialized_data.begin(), serialized_data.end());

    // sending the configuration packet (sending it REPEAT times -> repetition functionality)
    for (int i = 0; i < REPEAT; ++i) {
        std::cerr << "new config packet -> file id:" << fileId << "." << std::endl;
        this->session->sendingPackets(finalConfigPacket);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
}


// doing serialization to file in the path and send it in multiple chunks
void FileManagement::fileSender(uint32_t fileId, std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Error while opening the file..." << std::endl;
        return;
    }

    std::vector<char> buffer(this->chunk_size);
    uint64_t chunkId = 0;
    uint32_t packetType = 200; // meaning regular file
    std::vector<std::pair<uint32_t, std::vector<uint8_t>>> received_symbols;
    std::vector<uint8_t> finalRegularPacket;
    while(!file.eof())
    {
        file.read(buffer.data(), this->chunk_size);

        received_symbols = fec::encoder(std::string(buffer.begin(), buffer.end()),
                                        this->symbol_size, this->overhead);
        std::cout << "***********************************************************************" << std::endl << std::endl;
        std::cout << "number of symbols after encoding: " << received_symbols.size() << std::endl;

//        std::vector<std::pair<uint32_t, std::vector<uint8_t>>>
//                sliced_vec(received_symbols.begin() + 4, received_symbols.end());

        for (auto &encoded_symbol : received_symbols)
        {
            finalRegularPacket = createHeader(true, packetType, fileId, chunkId, encoded_symbol.first);
            finalRegularPacket.insert(finalRegularPacket.end(), encoded_symbol.second.begin(), encoded_symbol.second.end());
            std::cerr << "new regular packet -> file id:" << fileId << "." << std::endl;
            this->session->sendingPackets(finalRegularPacket);

            std::cout << "Packet number: " << fileId << "---" << chunkId << "---" << encoded_symbol.first << " sent!\n-> encoded data: " <<
            std::string(encoded_symbol.second.begin(), encoded_symbol.second.end())<< std::endl;

            finalRegularPacket.clear();
        }

        ++chunkId; // ready for the next chunk

        // reset vector elements
        received_symbols.clear(); // encoded vector
        buffer.assign(buffer.size(), 0); // file buffer
    }
    file.close();
}

// needs to pass in each file the fileId -> the config id is the same like the file id!
void FileManagement::directory_file_scanner(std::string path, std::string baseName)
{
    if (std::filesystem::is_directory(path)) {
        std::string currentDir =  baseName + std::filesystem::path(path).filename().string(); // new directory name
        int watch_fd = addPathToMonitor(this->inotify_fd, path);
        {
            std::lock_guard<std::mutex> lock(this->mutex_structure);
            this->map_path[watch_fd] = std::make_pair(path, currentDir);
        }
        this->createAndSendConfigPacket(fileId, path, currentDir);
        fileId++;
        for (const auto &entry: std::filesystem::directory_iterator(path)) {
            if (std::filesystem::is_regular_file(entry)) {
                std::string currentPath = entry.path().string();
                std::string currentFileName = currentDir + '/' + entry.path().filename().string();
                this->createAndSendConfigPacket(fileId, currentPath, currentFileName); // configPacket for each file
                this->fileSender(fileId, currentPath);
                fileId++;
            }
            else // if directory
            {
                std::string newPath = entry.path().string();
                directory_file_scanner(newPath, currentDir + '/');
            }
        }
    }
    else // sending a single file (not a directory)
    {
        std::string fileName = std::filesystem::path(path).filename().string(); // directory name
        int watch_fd = addPathToMonitor(this->inotify_fd, path);
        {
            std::lock_guard<std::mutex> lock(this->mutex_structure);
            this->map_path[watch_fd] = std::make_pair(path, fileName);
        }
        this->createAndSendConfigPacket(fileId, path, fileName); // configPacket first
        this->fileSender(fileId, path);
        fileId++;
        std::cout << "SENDING DATA DONE." << std::endl;
    }
}


int FileManagement::init_inotify_obj()
{
    int inotify_fd_ = inotify_init();
    if (inotify_fd_ == -1)
    {
        perror("inotify_init");
        return -1;
    }
    return inotify_fd_;
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
void FileManagement::monitorFunc(int inotify_fd, uint32_t chunkSize, uint32_t symbol_size, uint32_t overhead)
{
    std::cout << "IN THE MONITORING THREAD!!" << std::endl;
    std::cout << "Map Contents:" << std::endl;
    for (const auto& pair : this->map_path) {
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
            if (std::string(event->name).find(".swp") == std::string::npos &&
            std::string(event->name).find(".goutputstream") == std::string::npos)
            {
                if (event->mask & IN_CREATE || event->mask & IN_MODIFY) {
                    if (event->mask & IN_MODIFY)
                        std::cout << "File modified: " << event->name << std::endl;
                    else
                        std::cout << "File created: " << event->name << std::endl;

                    mutex_structure.lock();
                    std::string firstArg, secondArg;
                    firstArg = this->map_path[event->wd].first; // the parent path
                    secondArg = this->map_path[event->wd].second; // the base name

                    std::cout << "watch descriptor: " << event->wd << std::endl;
                    std::cout << "Parent path: " << firstArg << std::endl;
                    if(std::filesystem::is_directory(firstArg)) {
                        currentPath = this->map_path[event->wd].first + '/' + event->name; // the current path of the new file/dir
                        currentName = this->map_path[event->wd].second + '/' + event->name; // base name (if exist)
                    }
                    else {
                        currentPath = firstArg; // the current path of the new file/dir
                        currentName = secondArg;
                    }

                    std::cout << "currentPath: " << currentPath << std::endl;
                    std::cout << "currentName: " << currentName << std::endl;

                    this->createAndSendConfigPacket(fileId, currentPath, currentName);
                    if (std::filesystem::is_regular_file(currentPath))
                        this->fileSender(fileId, currentPath); // send the new file content after changing
                    else
                    {
                        // adding the new directory into the data structure
                        int watch_fd = addPathToMonitor(inotify_fd, currentPath);
                        this->map_path[watch_fd] = std::make_pair(currentPath, currentName);
                    }
                    mutex_structure.unlock();
                    fileId++;
                    startSending(); // sending the file after change
                }
            }
        }
    }
    else
    {
        std::cerr << "Failed while init inotify..." << std::endl;
    }
}

void FileManagement::startSending() {
    this->session->io_context.run();
}

FileManagement::~FileManagement() {
    std::cout << "---> FileManagement object is dead... of session number: " << this->session->getSessionNumber() << std::endl;
}
