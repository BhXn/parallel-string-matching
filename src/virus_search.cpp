#include "virus_search.hpp"
#include "matcher.hpp"
#include "utils.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

void run_virus_search(const std::string& input_dir, const std::string& output_path, int num_threads) {
    // 1. 读取所有病毒段文件（virus01.bin ~ virus10.bin）
    std::vector<FileView> virus_code;
    std::vector<std::string> virus_name;

    std::string virus_dir = input_dir + "/virus";
    std::vector<std::string> list_virus = list_all_files(virus_dir);

    std::sort(list_virus.begin(), list_virus.end());

    for (const std::string& path : list_virus) {
        FileView fv = read_file_view(path);
        if (fv.view.empty()) continue;  // 读失败则跳过
        virus_code.push_back(std::move(fv));
        virus_name.push_back(std::filesystem::path(path).filename().string());
    }

    // 2. 遍历软件目录（opencv-4.10.0）

    std::string soft_dir = input_dir + "/opencv-4.10.0";
    std::vector<std::string> files = list_all_files(soft_dir);

    // 3. 对每个文件匹配病毒
    std::vector<std::pair<std::string, std::vector<std::string>>> results;

    for (const std::string& file : files) {
        FileView file_view = read_file_view(file);
        std::string_view text = file_view.view;

        std::vector<std::string> hit;

        for (size_t i = 0; i < virus_code.size(); ++i) {

            const auto& viru = virus_code[i].view;

            auto pos = binary_match_parallel(text, viru, num_threads);

            if (!pos.empty()) {
                hit.push_back(virus_name[i]);
            }
        }

        if (!hit.empty()) {
            results.push_back({file, hit});
        }
    }

    std::ofstream fout(output_path);

    for (const auto& result : results) {

        fout << result.first;
        for (const std::string& viru : result.second) {
            fout << " " << viru;
        }
        fout << std::endl;
    }
}
