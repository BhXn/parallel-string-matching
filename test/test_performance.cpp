/**
 * Performance benchmark tool.
 * Usage: ./test_performance <data_root> [repeat=3]
 * data_root should contain document_retrieval/ and software_antivirus/ directories.
 */

#include "matcher.hpp"
#include "utils.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct DocData {
    FileView text_view;
    std::string_view text;
    std::vector<std::string> patterns;
};

struct VirusData {
    std::vector<FileView> viruses;
    std::vector<std::string> virus_names;
    std::vector<std::string> files;
};

using MatchFunc = std::vector<int> (*)(std::string_view, std::string_view, int);
using BinMatchFunc = std::vector<int> (*)(std::string_view, std::string_view, int);

double measure_seconds(const std::function<void()>& fn) {
    auto t0 = std::chrono::steady_clock::now();
    fn();
    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(t1 - t0).count();
}

DocData load_doc_data(const std::string& root) {
    DocData data;
    const std::string doc_path = root + "/document_retrieval/document.txt";
    const std::string target_path = root + "/document_retrieval/target.txt";
    data.text_view = read_file_view(doc_path);
    data.text = data.text_view.view;

    std::ifstream fin(target_path);
    std::string line;
    while (std::getline(fin, line)) {
        if (!line.empty()) data.patterns.push_back(line);
    }
    return data;
}

VirusData load_virus_data(const std::string& root) {
    VirusData data;
    const std::string virus_dir = root + "/software_antivirus/virus";
    const std::string soft_dir = root + "/software_antivirus/opencv-4.10.0";

    auto virus_files = list_all_files(virus_dir);
    std::sort(virus_files.begin(), virus_files.end());
    for (const auto& path : virus_files) {
        data.viruses.push_back(read_file_view(path));
        data.virus_names.push_back(std::filesystem::path(path).filename().string());
    }

    data.files = list_all_files(soft_dir);
    return data;
}

double bench_doc(const DocData& data, MatchFunc func, int threads, int repeat) {
    double total = 0.0;
    for (int i = 0; i < repeat; ++i) {
        total += measure_seconds([&]() {
            for (const auto& pattern : data.patterns) {
                (void)func(data.text, std::string_view(pattern), threads);
            }
        });
    }
    return total / repeat;
}

double bench_virus(const VirusData& data, BinMatchFunc func, int threads, int repeat) {
    double total = 0.0;
    for (int i = 0; i < repeat; ++i) {
        total += measure_seconds([&]() {
            for (const auto& file : data.files) {
                FileView file_view = read_file_view(file);
                std::string_view text(file_view.view);
                for (size_t idx = 0; idx < data.viruses.size(); ++idx) {
                    (void)func(text, data.viruses[idx].view, threads);
                }
            }
        });
    }
    return total / repeat;
}

template <typename Fn, typename Runner>
void print_table(const std::string& title, const std::vector<int>& thread_counts,
                 const std::vector<std::pair<std::string, Fn>>& funcs, Runner&& runner) {
    std::cout << "==== " << title << " ====\n";
    std::cout << "algorithm,threads,avg_seconds,speedup\n";
    std::cout << std::fixed << std::setprecision(4);
    for (const auto& item : funcs) {
        const auto& name = item.first;
        const auto& fn = item.second;
        double base = runner(fn, thread_counts.front());
        std::cout << name << "," << thread_counts.front() << "," << base << ",1.0\n";
        for (size_t i = 1; i < thread_counts.size(); ++i) {
            int th = thread_counts[i];
            double t = runner(fn, th);
            double speedup = (t > 0.0) ? (base / t) : 0.0;
            std::cout << name << "," << th << "," << t << "," << speedup << "\n";
        }
    }
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ./test_performance <data_root> [repeat=3]\n";
        return 1;
    }
    std::string data_root = argv[1];
    int repeat = (argc >= 3) ? std::stoi(argv[2]) : 3;

    std::vector<int> thread_counts = {1, 2, 4, 8, 10};

    DocData doc_data = load_doc_data(data_root);
    VirusData virus_data = load_virus_data(data_root);

    std::vector<std::pair<std::string, MatchFunc>> doc_funcs = {
        {"bf", match_parallel_bf}, {"kmp", match_parallel_kmp}, {"sunday", match_parallel_sunday},
        {"rk", match_parallel_rk}, {"bm", match_parallel_bm},
    };

    std::vector<std::pair<std::string, BinMatchFunc>> virus_funcs = {
        {"bf", binary_match_parallel_bf}, {"kmp", binary_match_parallel_kmp}, {"sunday", binary_match_parallel_sunday},
        {"rk", binary_match_parallel_rk}, {"bm", binary_match_parallel_bm},
    };

    print_table("document retrieval", thread_counts, doc_funcs,
                [&](const MatchFunc& fn, int th) { return bench_doc(doc_data, fn, th, repeat); });

    print_table("software antivirus", thread_counts, virus_funcs,
                [&](const BinMatchFunc& fn, int th) { return bench_virus(virus_data, fn, th, repeat); });

    return 0;
}
