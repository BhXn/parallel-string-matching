#include "utils.hpp"
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#ifdef __unix__
#include <sys/mman.h>
#include <unistd.h>
#endif

std::string read_text_file(const std::string& path) {
    std::ifstream fin(path);  // f=open(path)

    if (!fin.is_open()) {
        std::cout << "Fail to open file: " << path << std::endl;
        return "";
    }

    std::ostringstream oss;
    oss << fin.rdbuf();

    return oss.str();
}

std::vector<char> read_binary_file(const std::string& path) {
    std::ifstream fin(path, std::ios::binary);

    if (!fin.is_open()) {
        std::cout << "Fail to open file: " << path << std::endl;
        return {};
    }

    fin.seekg(0, std::ios::end);
    size_t size = fin.tellg();
    std::vector<char> buffer(size);

    fin.seekg(0);
    fin.read(buffer.data(), size);
    return buffer;
}

std::vector<std::string> list_all_files(const std::string& root_path) {
    std::vector<std::string> files;

    for (auto& entry : std::filesystem::recursive_directory_iterator(root_path)) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path().string());
        }
    }

    return files;
}

double now() {
    auto t = std::chrono::high_resolution_clock::now();
    double seconds = std::chrono::duration<double>(t.time_since_epoch()).count();

    return seconds;
}

FileView::~FileView() {
#ifdef __unix__
    if (mapped && mapping && size > 0) {
        ::munmap(mapping, size);
    }
    if (fd >= 0) {
        ::close(fd);
    }
#endif
}

FileView::FileView(FileView&& other) noexcept { *this = std::move(other); }

FileView& FileView::operator=(FileView&& other) noexcept {
    if (this == &other) return *this;
#ifdef __unix__
    if (mapped && mapping && size > 0) {
        ::munmap(mapping, size);
    }
    if (fd >= 0) {
        ::close(fd);
    }
#endif
    view = other.view;
    size = other.size;
    mapped = other.mapped;
    fd = other.fd;
    mapping = other.mapping;
    buffer = std::move(other.buffer);

    other.view = std::string_view{};
    other.size = 0;
    other.mapped = false;
    other.fd = -1;
    other.mapping = nullptr;
    return *this;
}

FileView read_file_view(const std::string& path, size_t mmap_threshold) {
    FileView fv;

    struct stat st{};
    if (stat(path.c_str(), &st) != 0) {
        std::cerr << "Fail to stat file: " << path << std::endl;
        return fv;
    }
    size_t file_size = static_cast<size_t>(st.st_size);
    fv.size = file_size;

#ifdef __unix__
    if (file_size >= mmap_threshold) {
        int fd = ::open(path.c_str(), O_RDONLY);
        if (fd >= 0) {
            void* addr = ::mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
            if (addr != MAP_FAILED) {
                fv.view = std::string_view(static_cast<const char*>(addr), file_size);
                fv.mapped = true;
                fv.fd = fd;
                fv.mapping = addr;
                return fv;
            } else {
                ::close(fd);
            }
        }
    }
#else
    (void)mmap_threshold;
#endif

    fv.buffer = read_binary_file(path);
    if (!fv.buffer.empty()) {
        fv.view = std::string_view(fv.buffer.data(), fv.buffer.size());
        fv.size = fv.buffer.size();
    }
    return fv;
}
