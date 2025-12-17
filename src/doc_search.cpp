#include "doc_search.hpp"
#include "matcher.hpp"
#include "utils.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

void run_doc_search(const std::string& input_dir, const std::string& output_path, int num_threads) {
    // 1. 读取 document.txt
    const std::string doc_path = input_dir + "/document.txt";
    const std::string target_path = input_dir + "/target.txt";
    FileView doc_view = read_file_view(doc_path);
    std::string text(doc_view.view);
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    //  2. 读取 target.txt（每行一个 pattern）
    std::vector<std::string> patterns;
    std::ifstream fin(target_path);
    std::string line;
    while (std::getline(fin, line)) {
        if (!line.empty()) patterns.push_back(line);
    }
    // 3. 对每个 pattern 调用 match_parallel

    std::vector<std::vector<int>> positions;

    for (const std::string& pattern : patterns) {
        std::vector<int> position = match_parallel(text, pattern, num_threads);
        positions.push_back(position);
    }

    // 4. 写入 output 文件
    std::ofstream fout(output_path);

    for (const std::vector<int>& p : positions) {
        fout << p.size();
        for (const int& pos : p) {
            fout << " " << pos;
        }
        fout << std::endl;
    }
}
