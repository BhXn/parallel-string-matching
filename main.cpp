#include "doc_search.hpp"
#include "matcher.hpp"
#include "utils.hpp"
#include "virus_search.hpp"
#include <filesystem>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Please run the project by: ./myapp <input_data_dir> <output_dir> [num_threads]\n";
        return 1;
    }

    std::string input_root = argv[1];
    std::string output_root = argv[2];
    int num_threads = (argc >= 4) ? std::stoi(argv[3]) : 10;

    // 创建输出目录
    std::filesystem::create_directories(output_root);

    std::cout << "Running document search...\n";
    double t =
        time_it(run_doc_search, input_root + "/document_retrieval", output_root + "/result_document.txt", num_threads);
    std::cout << "Document search done.\n";
    std::cout << "Doc search use time:" << t << "secs\n";

    std::cout << "Running virus scan...\n";
    t = time_it(run_virus_search, input_root + "/software_antivirus", output_root + "/result_software.txt",
                num_threads);
    std::cout << "Doc search use time:" << t << "secs\n";
    std::cout << "Virus scan done.\n";

    return 0;
}