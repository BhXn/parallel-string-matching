# Parallel String Matching Project

## 1. 项目简介

实现两个并行子任务：

1. **文档检索**：在 `document.txt` 中对 `target.txt` 里的多条模式串做并行匹配，输出次数与位置。
2. **软件病毒扫描**：在 `opencv-4.10.0` 目录中对所有文件做并行二进制匹配，输出含病毒的文件和病毒名。
3. **性能基准**：`test_performance` 对上述两个场景的 BF/KMP/Sunday/RK/BM 在多线程下进行耗时与加速比测试。

主要使用 **C++17 + std::thread**，匹配算法实现了 BF/KMP/Sunday/RK/BM，默认入口使用 BF。

## 2. 目录结构

```
code/
├── CMakeLists.txt          # 构建配置，产出 myapp / test_performance
├── main.cpp                # 主入口，串起文档检索与病毒扫描
├── include/                # 头文件
│     ├── matcher.hpp       # 串行/并行匹配算法（文本与二进制）
│     ├── doc_search.hpp    # 文档检索接口
│     ├── virus_search.hpp  # 病毒扫描接口
│     └── utils.hpp         # IO、计时、mmap 支持
├── src/                    # 实现
│     ├── matcher.cpp
│     ├── doc_search.cpp
│     ├── virus_search.cpp
│     └── utils.cpp
├── test/test_performance.cpp # 性能基准工具
└── output/                 # 示例输出（程序运行时自动创建目录）
      ├── result_document.txt
      └── result_software.txt
```

## 3. 核心实现说明

- **并行策略**：`matcher.cpp` 将文本按线程数切分为等长块，每块向右额外拓展 `pattern_len-1` 避免跨块遗漏；子线程返回的命中位置合并后排序去重。
- **算法选择**：提供 BF/KMP/Sunday/RK/BM 的串行与并行版本，二进制匹配同样覆盖。默认 `match_parallel` / `binary_match_parallel` 走 BF，可按需替换为其他版本。
- **文档检索**：`run_doc_search` 读取整份文档和所有目标串，逐一调用并行匹配，输出 `count pos...`。
- **病毒扫描**：`run_virus_search` 先递归读取 `virus/` 下的所有病毒片段，再递归遍历 `opencv-4.10.0/` 的每个文件并并行匹配，命中则记录 `文件路径 病毒名...`。
- **IO/性能**：常规 IO 由 `read_text_file` / `read_binary_file` 完成；`FileView` 在类 Unix 下大文件自动使用 mmap（基准工具中使用）。

## 4. 编译（CMake）

```
cd code
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

生成的可执行文件：

- `build/myapp`：主程序
- `build/test_performance`：性能基准（可选）

## 5. 运行主程序

```
./myapp <input_data_dir> <output_dir> [num_threads]
```

参数说明：

- `<input_data_dir>`：数据根目录，要求包含 `document_retrieval/` 与 `software_antivirus/`。
- `<output_dir>`：输出目录（不存在会自动创建）。
- `[num_threads]`：可选并行线程数，默认 10。

示例（假设 `data/` 与 `code/` 同级）：

```
cd code/build
./myapp ../../data ../output 20
```

输出：

- `result_document.txt`：每行 `match_count pos1 pos2 ...`，使用 0-based 偏移。
- `result_software.txt`：`文件相对路径 病毒1 病毒2 ...`。

## 6. 性能基准工具

```
./test_performance <data_root> [repeat=3]
```

说明：

- 预设线程数：1/2/4/8/10，可修改 `test/test_performance.cpp` 中的 `thread_counts`。
- 输出 CSV 表头为 `algorithm,threads,avg_seconds,speedup`，便于重定向到文件或导入表格工具。
- 文档与病毒场景分别基于真实数据运行；大文件使用 `FileView`/mmap 以降低 IO 开销。

示例输出片段：

```
==== document retrieval ====
algorithm,threads,avg_seconds,speedup
bf,1,12.3456,1.0
bf,2,6.1234,2.0
...
```

## 7. 依赖

- C++17
- CMake ≥ 3.15
- pthread（macOS 默认自带，Linux 已在 CMake 中链接 `Threads::Threads`）

## 8. 作者

赵旭令，南京大学人工智能专业，2025 年课程 Project 提交。
