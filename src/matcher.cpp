#include "matcher.hpp"
#include <algorithm>
#include <thread>

using StrView = std::string_view;
using MatchFnPtr = std::vector<int> (*)(StrView, StrView);

std::vector<int> match_single_bf(StrView text, StrView pattern) {

    std::vector<int> positions;

    const size_t n = text.size();
    const size_t m = pattern.size();

    if (m == 0 || n < m) return positions;

    for (size_t i = 0; i + m <= n; i++) {
        bool flag = true;
        for (size_t j = 0; j < m; j++) {
            if (text[i + j] != pattern[j]) {
                flag = false;
                break;
            }
        }
        if (flag) {
            positions.push_back(static_cast<int>(i));
        }
    }

    return positions;
}

std::vector<int> match_single(StrView text, StrView pattern) { return match_single_bf(text, pattern); }

std::vector<int> compute_lps(StrView pattern) {
    int m = static_cast<int>(pattern.size());
    std::vector<int> lps(m, 0);

    int len = 0;
    int i = 1;

    while (i < m) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                i++;
            }
        }
    }

    return lps;
}

std::vector<int> match_single_kmp(StrView text, StrView pattern) {
    std::vector<int> positions;

    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());

    if (m == 0 || n < m) return positions;

    std::vector<int> lps = compute_lps(pattern);

    int i = 0;
    int j = 0;

    while (i < n) {

        if (text[i] == pattern[j]) {
            i++;
            j++;

            if (j == m) {
                positions.push_back(i - m);
                j = lps[j - 1];
            }

        } else {  // mismatch

            if (j > 0) {
                j = lps[j - 1];
            } else {
                i++;
            }
        }
    }

    return positions;
}

std::vector<int> match_single_sunday(StrView text, StrView pattern) {
    std::vector<int> positions;

    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());

    if (m == 0 || n < m) return positions;

    std::vector<int> shift(256, m + 1);

    for (int i = 0; i < m; i++) {
        shift[(unsigned char)pattern[i]] = m - i;
    }

    int i = 0;

    while (i <= n - m) {
        bool flag = true;

        for (int j = 0; j < m; j++) {
            if (text[i + j] != pattern[j]) {
                flag = false;
                break;
            }
        }

        if (flag) {
            positions.push_back(i);
        }

        if (i + m >= n) {
            break;
        }
        i += shift[(unsigned char)text[i + m]];
    }

    return positions;
}

using ull = unsigned long long;
const ull base = 131;

ull compute_hash(StrView s, int m) {
    ull h = 0;
    for (int i = 0; i < m; i++) {
        h = h * base + (unsigned char)s[i];
    }
    return h;
}

ull compute_power(int m) {
    ull p = 1;
    for (int i = 1; i < m; i++) {
        p *= base;
    }
    return p;
}

ull roll_hash(ull old_hash, char old_char, char new_char, ull power) {
    // 去掉最高位字符 old_char 的贡献
    ull h = old_hash - (ull)(unsigned char)old_char * power;
    // 左移（乘 base）
    h = h * base;
    // 加入新字符
    h += (unsigned char)new_char;
    return h;
}

std::vector<int> match_single_rk(StrView text, StrView pattern) {
    std::vector<int> positions;

    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());

    if (m == 0 || n < m) return positions;

    ull pattern_hash = compute_hash(pattern, m);
    ull text_hash = compute_hash(text, m);
    ull power = compute_power(m);

    int i = 0;
    while (i <= n - m) {
        if (text_hash == pattern_hash) {
            bool flag = true;
            for (int j = 0; j < m; j++) {
                if (text[i + j] != pattern[j]) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                positions.push_back(i);
            }
        }
        if (i == n - m) break;
        text_hash = roll_hash(text_hash, text[i], text[i + m], power);  // m>0因此i+1不会越界
        i++;
    }

    return positions;
}

std::vector<int> match_single_bm_bc(StrView text, StrView pattern) {
    std::vector<int> positions;

    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());
    if (m == 0 || n < m) return positions;

    std::vector<int> last(256, -1);
    for (int k = 0; k < m; ++k) {
        last[(unsigned char)pattern[k]] = k;
    }

    int i = 0;
    while (i <= n - m) {
        int j = m - 1;

        while (j >= 0 && pattern[j] == text[i + j]) {
            --j;
        }

        if (j < 0) {

            positions.push_back(i);
            i += 1;
        } else {
            unsigned char bad = (unsigned char)text[i + j];
            int lo = last[bad];

            int shift = j - lo;
            if (shift < 1) shift = 1;

            i += shift;
        }
    }

    return positions;
}

void build_bad_char(StrView pattern, std::vector<int>& bad_char) {
    int m = static_cast<int>(pattern.size());
    bad_char.assign(256, -1);
    for (int i = 0; i < m; ++i) {
        bad_char[(unsigned char)pattern[i]] = i;
    }
}

// 构建好后缀（suffix / prefix）表
// suffix[k]  = 长度为 k 的后缀子串（好后缀的“某个后缀”）在 pattern 中最右匹配的起始下标（不含末尾这次）
// prefix[k]  = 长度为 k 的后缀子串是否同时是 pattern 的前缀
void build_good_suffix(StrView pattern, std::vector<int>& suffix, std::vector<bool>& prefix) {
    int m = static_cast<int>(pattern.size());
    suffix.assign(m, -1);
    prefix.assign(m, false);

    // 遍历 pattern[0..m-2]，尝试和以 pattern[m-1] 结尾的后缀对齐
    for (int i = 0; i < m - 1; ++i) {
        int j = i;
        int k = 0;  // 当前匹配的后缀长度

        // 从 pattern[i] 和 pattern[m-1] 开始，向左比较
        while (j >= 0 && pattern[j] == pattern[m - 1 - k]) {
            --j;
            ++k;
            // 记录：长度为 k 的后缀子串，在模式中最右匹配的起始位置是 j+1
            suffix[k] = j + 1;
        }

        // 如果 j < 0，说明这一段前缀本身就是某个后缀（即“后缀又是前缀”）
        if (j < 0) {
            prefix[k] = true;
        }
    }
}

// 根据好后缀表计算在 mismatch 位置 j 时的位移
int move_by_good_suffix(int j, int m, const std::vector<int>& suffix, const std::vector<bool>& prefix) {
    int k = m - 1 - j;  // 好后缀长度（已经匹配成功的尾巴长度）

    if (k <= 0) return 0;  // 没有好后缀时不由好后缀规则决定位移

    // 情况 1：存在长度为 k 的好后缀的“另一处出现”
    if (suffix[k] != -1) {
        return j + 1 - suffix[k];
    }

    // 情况 2：不存在完整的好后缀匹配，尝试好后缀的“更短后缀”是否为前缀
    for (int r = k - 1; r > 0; --r) {
        if (prefix[r]) {
            return m - r;
        }
    }

    // 情况 3：都不存在，整串右移 m 位
    return m;
}

// 完整 BM：坏字符 + 好后缀
std::vector<int> match_single_bm(StrView text, StrView pattern) {
    std::vector<int> positions;

    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());
    if (m == 0 || n < m) return positions;

    // 1. 构建坏字符表
    std::vector<int> bad_char;
    build_bad_char(pattern, bad_char);

    // 2. 构建好后缀表
    std::vector<int> suffix;
    std::vector<bool> prefix;
    build_good_suffix(pattern, suffix, prefix);

    // 3. 主循环：i 为窗口左端
    int i = 0;
    while (i <= n - m) {
        int j = m - 1;

        // 从右往左匹配
        while (j >= 0 && pattern[j] == text[i + j]) {
            --j;
        }

        if (j < 0) {
            // 匹配成功
            positions.push_back(i);
            // 这里简单起见，右移 1（也可以用好后缀/整串位移优化）
            i += 1;
        } else {
            // 坏字符规则
            unsigned char bad = (unsigned char)text[i + j];
            int last_pos = bad_char[bad];  // 该坏字符在 pattern 中最后一次出现的位置（-1 表示不存在）
            int shift_bc = j - last_pos;   // 标准坏字符位移
            if (shift_bc < 1) shift_bc = 1;

            // 好后缀规则
            int shift_gs = move_by_good_suffix(j, m, suffix, prefix);

            // 取两者较大者
            int shift = std::max(shift_bc, shift_gs);
            i += shift;
        }
    }

    return positions;
}

namespace {
template <typename MatchFunc>
std::vector<int> parallel_match_impl(StrView text, StrView pattern, int num_threads, MatchFunc match_func) {
    std::vector<int> positions;

    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());

    if (m == 0 || n < m) return positions;

    num_threads = std::min(num_threads, n / std::max(1, m));
    if (num_threads <= 0) num_threads = 1;

    int chunk_size = n / num_threads;

    std::vector<std::vector<int>> all_positions(num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
        int start = thread_id * chunk_size;
        int end = (thread_id == num_threads - 1) ? n : (thread_id + 1) * chunk_size;
        end = std::min(end + (m - 1), n);

        threads.emplace_back([&, thread_id, start, end]() {
            StrView segment = text.substr(start, end - start);
            auto local_pos = match_func(segment, pattern);
            for (int p : local_pos) {
                all_positions[thread_id].push_back(start + p);
            }
        });
    }

    for (auto& th : threads) th.join();

    for (auto& vec : all_positions) {
        positions.insert(positions.end(), vec.begin(), vec.end());
    }

    std::sort(positions.begin(), positions.end());
    positions.erase(std::unique(positions.begin(), positions.end()), positions.end());

    return positions;
}
}  // namespace

std::vector<int> match_parallel(StrView text, StrView pattern, int num_threads) {
    return match_parallel_bf(text, pattern, num_threads);
}

std::vector<int> match_parallel_bf(StrView text, StrView pattern, int num_threads) {
    return parallel_match_impl(text, pattern, num_threads, static_cast<MatchFnPtr>(match_single_bf));
}

std::vector<int> match_parallel_kmp(StrView text, StrView pattern, int num_threads) {
    return parallel_match_impl(text, pattern, num_threads, static_cast<MatchFnPtr>(match_single_kmp));
}

std::vector<int> match_parallel_sunday(StrView text, StrView pattern, int num_threads) {
    return parallel_match_impl(text, pattern, num_threads, static_cast<MatchFnPtr>(match_single_sunday));
}

std::vector<int> match_parallel_rk(StrView text, StrView pattern, int num_threads) {
    return parallel_match_impl(text, pattern, num_threads, static_cast<MatchFnPtr>(match_single_rk));
}

std::vector<int> match_parallel_bm(StrView text, StrView pattern, int num_threads) {
    return parallel_match_impl(text, pattern, num_threads, static_cast<MatchFnPtr>(match_single_bm));
}

// 兼容 std::string 的包装
std::vector<int> match_single(const std::string& text, const std::string& pattern) {
    return match_single(StrView(text), StrView(pattern));
}
std::vector<int> match_single_bf(const std::string& text, const std::string& pattern) {
    return match_single_bf(StrView(text), StrView(pattern));
}
std::vector<int> match_single_kmp(const std::string& text, const std::string& pattern) {
    return match_single_kmp(StrView(text), StrView(pattern));
}
std::vector<int> match_single_sunday(const std::string& text, const std::string& pattern) {
    return match_single_sunday(StrView(text), StrView(pattern));
}
std::vector<int> match_single_rk(const std::string& text, const std::string& pattern) {
    return match_single_rk(StrView(text), StrView(pattern));
}
std::vector<int> match_single_bm(const std::string& text, const std::string& pattern) {
    return match_single_bm(StrView(text), StrView(pattern));
}

std::vector<int> match_parallel(const std::string& text, const std::string& pattern, int num_threads) {
    return match_parallel(StrView(text), StrView(pattern), num_threads);
}
std::vector<int> match_parallel_bf(const std::string& text, const std::string& pattern, int num_threads) {
    return match_parallel_bf(StrView(text), StrView(pattern), num_threads);
}
std::vector<int> match_parallel_kmp(const std::string& text, const std::string& pattern, int num_threads) {
    return match_parallel_kmp(StrView(text), StrView(pattern), num_threads);
}
std::vector<int> match_parallel_sunday(const std::string& text, const std::string& pattern, int num_threads) {
    return match_parallel_sunday(StrView(text), StrView(pattern), num_threads);
}
std::vector<int> match_parallel_rk(const std::string& text, const std::string& pattern, int num_threads) {
    return match_parallel_rk(StrView(text), StrView(pattern), num_threads);
}
std::vector<int> match_parallel_bm(const std::string& text, const std::string& pattern, int num_threads) {
    return match_parallel_bm(StrView(text), StrView(pattern), num_threads);
}

std::vector<int> binary_match_single(StrView text, StrView pattern) {
    std::vector<int> positions;

    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());

    if (m == 0 || n < m) return positions;

    for (int i = 0; i <= n - m; i++) {
        bool flag = true;
        for (int j = 0; j < m; j++) {
            if (text[i + j] != pattern[j]) {
                flag = false;
                break;
            }
        }
        if (flag) {
            positions.push_back(i);
        }
    }
    return positions;
}

std::vector<int> binary_match_single(const std::vector<char>& text, const std::vector<char>& pattern) {
    return binary_match_single(StrView(text.data(), text.size()), StrView(pattern.data(), pattern.size()));
}

std::vector<int> binary_match_single_kmp(StrView text, StrView pattern) {
    std::vector<int> positions;

    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());
    if (m == 0 || n < m) return positions;

    // 构建 lps
    std::vector<int> lps(m, 0);
    for (int i = 1, len = 0; i < m;) {
        if (pattern[i] == pattern[len]) {
            lps[i++] = ++len;
        } else if (len > 0) {
            len = lps[len - 1];
        } else {
            lps[i++] = 0;
        }
    }

    for (int i = 0, j = 0; i < n;) {
        if (text[i] == pattern[j]) {
            ++i;
            ++j;
            if (j == m) {
                positions.push_back(i - m);
                j = lps[j - 1];
            }
        } else if (j > 0) {
            j = lps[j - 1];
        } else {
            ++i;
        }
    }

    return positions;
}

std::vector<int> binary_match_single_kmp(const std::vector<char>& text, const std::vector<char>& pattern) {
    return binary_match_single_kmp(StrView(text.data(), text.size()), StrView(pattern.data(), pattern.size()));
}

std::vector<int> binary_match_single_sunday(StrView text, StrView pattern) {
    std::vector<int> positions;
    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());
    if (m == 0 || n < m) return positions;

    std::vector<int> shift(256, m + 1);
    for (int i = 0; i < m; ++i) {
        shift[(unsigned char)pattern[i]] = m - i;
    }

    for (int i = 0; i <= n - m;) {
        bool ok = true;
        for (int j = 0; j < m; ++j) {
            if (text[i + j] != pattern[j]) {
                ok = false;
                break;
            }
        }
        if (ok) positions.push_back(i);
        if (i + m >= n) break;
        i += shift[(unsigned char)text[i + m]];
    }
    return positions;
}

std::vector<int> binary_match_single_sunday(const std::vector<char>& text, const std::vector<char>& pattern) {
    return binary_match_single_sunday(StrView(text.data(), text.size()), StrView(pattern.data(), pattern.size()));
}

std::vector<int> binary_match_single_rk(StrView text, StrView pattern) {
    std::vector<int> positions;

    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());
    if (m == 0 || n < m) return positions;

    auto compute_hash_vec = [&](StrView arr, int start, int len) {
        ull h = 0;
        for (int i = 0; i < len; ++i) {
            h = h * base + (unsigned char)arr[start + i];
        }
        return h;
    };

    ull pattern_hash = compute_hash_vec(pattern, 0, m);
    ull text_hash = compute_hash_vec(text, 0, m);

    ull power = 1;
    for (int i = 1; i < m; ++i) power *= base;

    for (int i = 0; i <= n - m; ++i) {
        if (text_hash == pattern_hash) {
            bool ok = true;
            for (int j = 0; j < m; ++j) {
                if (text[i + j] != pattern[j]) {
                    ok = false;
                    break;
                }
            }
            if (ok) positions.push_back(i);
        }
        if (i == n - m) break;
        text_hash = roll_hash(text_hash, text[i], text[i + m], power);
    }
    return positions;
}

std::vector<int> binary_match_single_rk(const std::vector<char>& text, const std::vector<char>& pattern) {
    return binary_match_single_rk(StrView(text.data(), text.size()), StrView(pattern.data(), pattern.size()));
}

std::vector<int> binary_match_single_bm(StrView text, StrView pattern) {
    std::vector<int> positions;
    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());
    if (m == 0 || n < m) return positions;

    std::vector<int> bad_char(256, -1);
    for (int i = 0; i < m; ++i) bad_char[(unsigned char)pattern[i]] = i;

    std::vector<int> suffix(m, -1);
    std::vector<bool> prefix(m, false);
    // build_good_suffix for vector<char>
    for (int i = 0; i < m - 1; ++i) {
        int j = i;
        int k = 0;
        while (j >= 0 && pattern[j] == pattern[m - 1 - k]) {
            --j;
            ++k;
            suffix[k] = j + 1;
        }
        if (j < 0) prefix[k] = true;
    }

    auto move_by_good_suffix_vec = [&](int j) {
        int k = m - 1 - j;
        if (k <= 0) return 0;
        if (suffix[k] != -1) return j + 1 - suffix[k];
        for (int r = k - 1; r > 0; --r) {
            if (prefix[r]) return m - r;
        }
        return m;
    };

    for (int i = 0; i <= n - m;) {
        int j = m - 1;
        while (j >= 0 && pattern[j] == text[i + j]) --j;
        if (j < 0) {
            positions.push_back(i);
            i += 1;
        } else {
            unsigned char bad = (unsigned char)text[i + j];
            int shift_bc = j - bad_char[bad];
            if (shift_bc < 1) shift_bc = 1;
            int shift_gs = move_by_good_suffix_vec(j);
            i += std::max(shift_bc, shift_gs);
        }
    }
    return positions;
}

std::vector<int> binary_match_single_bm(const std::vector<char>& text, const std::vector<char>& pattern) {
    return binary_match_single_bm(StrView(text.data(), text.size()), StrView(pattern.data(), pattern.size()));
}

namespace {
template <typename MatchFunc>
std::vector<int> parallel_binary_impl(StrView text, StrView pattern, int num_threads, MatchFunc match_func) {
    std::vector<int> positions;

    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());
    if (m == 0 || n < m) return positions;

    num_threads = std::min(num_threads, n / std::max(1, m));
    if (num_threads <= 0) num_threads = 1;

    int chunk_size = n / num_threads;

    std::vector<std::vector<int>> all_positions(num_threads);
    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
        int start = thread_id * chunk_size;
        int end = (thread_id == num_threads - 1) ? n : (thread_id + 1) * chunk_size;
        end = std::min(end + (m - 1), n);

        threads.emplace_back([&, thread_id, start, end]() {
            StrView segment = text.substr(start, end - start);
            auto local_pos = match_func(segment, pattern);
            for (int p : local_pos) all_positions[thread_id].push_back(start + p);
        });
    }

    for (auto& th : threads) th.join();

    for (auto& vec : all_positions) positions.insert(positions.end(), vec.begin(), vec.end());
    std::sort(positions.begin(), positions.end());
    positions.erase(std::unique(positions.begin(), positions.end()), positions.end());
    return positions;
}
}  // namespace

std::vector<int> binary_match_parallel(StrView text, StrView pattern, int num_threads) {
    return binary_match_parallel_bf(text, pattern, num_threads);
}

std::vector<int> binary_match_parallel_bf(StrView text, StrView pattern, int num_threads) {
    return parallel_binary_impl(text, pattern, num_threads, static_cast<MatchFnPtr>(binary_match_single));
}

std::vector<int> binary_match_parallel_kmp(StrView text, StrView pattern, int num_threads) {
    return parallel_binary_impl(text, pattern, num_threads, static_cast<MatchFnPtr>(binary_match_single_kmp));
}

std::vector<int> binary_match_parallel_sunday(StrView text, StrView pattern, int num_threads) {
    return parallel_binary_impl(text, pattern, num_threads, static_cast<MatchFnPtr>(binary_match_single_sunday));
}

std::vector<int> binary_match_parallel_rk(StrView text, StrView pattern, int num_threads) {
    return parallel_binary_impl(text, pattern, num_threads, static_cast<MatchFnPtr>(binary_match_single_rk));
}

std::vector<int> binary_match_parallel_bm(StrView text, StrView pattern, int num_threads) {
    return parallel_binary_impl(text, pattern, num_threads, static_cast<MatchFnPtr>(binary_match_single_bm));
}

std::vector<int> binary_match_parallel(const std::vector<char>& text, const std::vector<char>& pattern,
                                       int num_threads) {
    return binary_match_parallel(StrView(text.data(), text.size()), StrView(pattern.data(), pattern.size()),
                                 num_threads);
}

std::vector<int> binary_match_parallel_bf(const std::vector<char>& text, const std::vector<char>& pattern,
                                          int num_threads) {
    return binary_match_parallel_bf(StrView(text.data(), text.size()), StrView(pattern.data(), pattern.size()),
                                    num_threads);
}

std::vector<int> binary_match_parallel_kmp(const std::vector<char>& text, const std::vector<char>& pattern,
                                           int num_threads) {
    return binary_match_parallel_kmp(StrView(text.data(), text.size()), StrView(pattern.data(), pattern.size()),
                                     num_threads);
}

std::vector<int> binary_match_parallel_sunday(const std::vector<char>& text, const std::vector<char>& pattern,
                                              int num_threads) {
    return binary_match_parallel_sunday(StrView(text.data(), text.size()), StrView(pattern.data(), pattern.size()),
                                        num_threads);
}

std::vector<int> binary_match_parallel_rk(const std::vector<char>& text, const std::vector<char>& pattern,
                                          int num_threads) {
    return binary_match_parallel_rk(StrView(text.data(), text.size()), StrView(pattern.data(), pattern.size()),
                                    num_threads);
}

std::vector<int> binary_match_parallel_bm(const std::vector<char>& text, const std::vector<char>& pattern,
                                          int num_threads) {
    return binary_match_parallel_bm(StrView(text.data(), text.size()), StrView(pattern.data(), pattern.size()),
                                    num_threads);
}
