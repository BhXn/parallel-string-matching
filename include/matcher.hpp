#pragma once
#include <string>
#include <string_view>
#include <vector>

// string_view 版本（可零拷贝）
std::vector<int> match_single(std::string_view text, std::string_view pattern);
std::vector<int> match_single_bf(std::string_view text, std::string_view pattern);
std::vector<int> match_single_kmp(std::string_view text, std::string_view pattern);
std::vector<int> match_single_sunday(std::string_view text, std::string_view pattern);
std::vector<int> match_single_rk(std::string_view text, std::string_view pattern);
std::vector<int> match_single_bm(std::string_view text, std::string_view pattern);

std::vector<int> match_parallel(std::string_view text, std::string_view pattern, int num_threads);
std::vector<int> match_parallel_bf(std::string_view text, std::string_view pattern, int num_threads);
std::vector<int> match_parallel_kmp(std::string_view text, std::string_view pattern, int num_threads);
std::vector<int> match_parallel_sunday(std::string_view text, std::string_view pattern, int num_threads);
std::vector<int> match_parallel_rk(std::string_view text, std::string_view pattern, int num_threads);
std::vector<int> match_parallel_bm(std::string_view text, std::string_view pattern, int num_threads);

// 兼容旧接口（std::string 输入）
std::vector<int> match_single(const std::string& text, const std::string& pattern);
std::vector<int> match_single_bf(const std::string& text, const std::string& pattern);
std::vector<int> match_single_kmp(const std::string& text, const std::string& pattern);
std::vector<int> match_single_sunday(const std::string& text, const std::string& pattern);
std::vector<int> match_single_rk(const std::string& text, const std::string& pattern);
std::vector<int> match_single_bm(const std::string& text, const std::string& pattern);

std::vector<int> match_parallel(const std::string& text, const std::string& pattern, int num_threads);
std::vector<int> match_parallel_bf(const std::string& text, const std::string& pattern, int num_threads);
std::vector<int> match_parallel_kmp(const std::string& text, const std::string& pattern, int num_threads);
std::vector<int> match_parallel_sunday(const std::string& text, const std::string& pattern, int num_threads);
std::vector<int> match_parallel_rk(const std::string& text, const std::string& pattern, int num_threads);
std::vector<int> match_parallel_bm(const std::string& text, const std::string& pattern, int num_threads);

// 二进制匹配（string_view 版本 + 兼容 vector 版本）
std::vector<int> binary_match_single(std::string_view text, std::string_view pattern);
std::vector<int> binary_match_single_kmp(std::string_view text, std::string_view pattern);
std::vector<int> binary_match_single_sunday(std::string_view text, std::string_view pattern);
std::vector<int> binary_match_single_rk(std::string_view text, std::string_view pattern);
std::vector<int> binary_match_single_bm(std::string_view text, std::string_view pattern);

std::vector<int> binary_match_parallel(std::string_view text, std::string_view pattern, int num_threads);
std::vector<int> binary_match_parallel_bf(std::string_view text, std::string_view pattern, int num_threads);
std::vector<int> binary_match_parallel_kmp(std::string_view text, std::string_view pattern, int num_threads);
std::vector<int> binary_match_parallel_sunday(std::string_view text, std::string_view pattern, int num_threads);
std::vector<int> binary_match_parallel_rk(std::string_view text, std::string_view pattern, int num_threads);
std::vector<int> binary_match_parallel_bm(std::string_view text, std::string_view pattern, int num_threads);

std::vector<int> binary_match_single(const std::vector<char>& text, const std::vector<char>& pattern);
std::vector<int> binary_match_single_kmp(const std::vector<char>& text, const std::vector<char>& pattern);
std::vector<int> binary_match_single_sunday(const std::vector<char>& text, const std::vector<char>& pattern);
std::vector<int> binary_match_single_rk(const std::vector<char>& text, const std::vector<char>& pattern);
std::vector<int> binary_match_single_bm(const std::vector<char>& text, const std::vector<char>& pattern);

std::vector<int> binary_match_parallel(const std::vector<char>& text, const std::vector<char>& pattern,
                                       int num_threads);
std::vector<int> binary_match_parallel_bf(const std::vector<char>& text, const std::vector<char>& pattern,
                                          int num_threads);
std::vector<int> binary_match_parallel_kmp(const std::vector<char>& text, const std::vector<char>& pattern,
                                           int num_threads);
std::vector<int> binary_match_parallel_sunday(const std::vector<char>& text, const std::vector<char>& pattern,
                                              int num_threads);
std::vector<int> binary_match_parallel_rk(const std::vector<char>& text, const std::vector<char>& pattern,
                                          int num_threads);
std::vector<int> binary_match_parallel_bm(const std::vector<char>& text, const std::vector<char>& pattern,
                                          int num_threads);
