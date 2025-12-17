#pragma once
#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

std::string read_text_file(const std::string& path);

std::vector<char> read_binary_file(const std::string& path);

std::vector<std::string> list_all_files(const std::string& root_path);

double now();

double now();

struct FileView {
    std::string_view view{};
    size_t size{0};
    bool mapped{false};
    int fd{-1};
    void* mapping{nullptr};
    std::vector<char> buffer;

    FileView() = default;
    ~FileView();
    FileView(FileView&& other) noexcept;
    FileView& operator=(FileView&& other) noexcept;

    FileView(const FileView&) = delete;
    FileView& operator=(const FileView&) = delete;
};

// 自动根据大小选择 mmap（大文件）或常规读（小文件），默认阈值 8MB。
FileView read_file_view(const std::string& path, size_t mmap_threshold = 8 * 1024 * 1024);

template <typename Func, typename... Args> double time_it(Func func, Args&&... args) {
    double t0 = now();
    func(std::forward<Args>(args)...);
    double t1 = now();
    return t1 - t0;
}
