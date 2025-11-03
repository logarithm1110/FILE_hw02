#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <chrono>
#include "fileio.h"
#include "format.h"
#include "preprocess.h"
#include "bitio.h"
#include "lzw_compress.h"
#include "lzw_decompress.h"

// Parsed args 结构体
struct ParsedArgs {
    std::string src;
    std::string dst;
    std::string mode; // "zip" or "unzip"
};

// 打印用法
void printUsage(const char* prog) {
    std::cerr << "Usage: " << prog << " {src} {dst} {zip|unzip}\n";
}

// 检查文件是否存在（尝试以二进制打开）
bool fileExists(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return in.good();
}

// 解析命令行并校验
bool parseArgs(int argc, char* argv[], ParsedArgs& parsedArgs) {
    if (argc != 4) {
        printUsage(argv[0]);
        std::cerr << "Error: expected 3 parameters, got " << (argc - 1) << "\n";
        return false;
    }
    parsedArgs.src = argv[1];
    parsedArgs.dst = argv[2];
    parsedArgs.mode = argv[3];

    if (parsedArgs.mode != "zip" && parsedArgs.mode != "unzip") {
        std::cerr << "Error: unknown mode '" << parsedArgs.mode << "'. Only 'zip' or 'unzip' allowed.\n";
        return false;
    }

    if (!fileExists(parsedArgs.src)) {
        std::cerr << "Error: source file '" << parsedArgs.src << "' does not exist or cannot be opened.\n";
        return false;
    }

    return true;
}

// 压缩函数
bool compressFile(const std::string& src_path, const std::string& dst_path) {
    auto start_time = std::chrono::high_resolution_clock::now();

    // 1. 获取文件大小但不读取全部内容
    std::ifstream src_file(src_path, std::ios::binary);
    if (!src_file) {
        std::cerr << "Error: cannot open source file for reading\n";
        return false;
    }

    src_file.seekg(0, std::ios::end);
    uint64_t original_size = static_cast<uint64_t>(src_file.tellg());
    src_file.seekg(0, std::ios::beg);

    // 2. 禁用预处理（暂时）以提高速度
    std::cout << "Original size: " << original_size << " bytes\n";

    // 3. 写入头部（不使用预处理）
    std::ofstream dst_file(dst_path, std::ios::binary | std::ios::trunc);
    if (!dst_file) {
        std::cerr << "Error: cannot open destination file for writing\n";
        return false;
    }

    ArchiveHeader header;
    header.original_size = original_size;
    header.setPreprocessing(false);  // 禁用预处理
    header.max_code_width = 12;

    if (!writeHeader(dst_file, header)) {
        std::cerr << "Error: failed to write header\n";
        return false;
    }

    // 4. 直接 LZW 压缩（不经过预处理）
    BitWriter bit_writer(dst_file);
    LZWCompressor compressor(LZWCompressOptions(9, 12));

    if (!compressor.compressStream(src_file, bit_writer)) {
        std::cerr << "Error: LZW compression failed\n";
        return false;
    }

    bit_writer.flush();
    src_file.close();
    dst_file.close();

    // 5. 检查结果
    std::ifstream compressed_file(dst_path, std::ios::binary | std::ios::ate);
    uint64_t compressed_size = static_cast<uint64_t>(compressed_file.tellg());
    compressed_file.close();

    double compression_ratio = static_cast<double>(compressed_size) / static_cast<double>(original_size);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Compression complete!\n";
    std::cout << "Compressed size: " << compressed_size << " bytes\n";
    std::cout << "Compression ratio: " << (compression_ratio * 100) << "%\n";
    std::cout << "Time taken: " << duration.count() << " ms\n";

    return compression_ratio < 0.8;
}

// 解压函数
bool decompressFile(const std::string& src_path, const std::string& dst_path) {
    auto start_time = std::chrono::high_resolution_clock::now();

    // 1. 打开压缩文件并读取头部
    std::ifstream src_file(src_path, std::ios::binary);
    if (!src_file) {
        std::cerr << "Error: cannot open compressed file for reading\n";
        return false;
    }

    ArchiveHeader header;
    if (!readHeader(src_file, header)) {
        std::cerr << "Error: failed to read header\n";
        return false;
    }

    if (!headerMagicOk(header)) {
        std::cerr << "Error: invalid file format\n";
        return false;
    }

    std::cout << "Archive info: version=" << int(header.version)
        << ", original_size=" << header.original_size
        << ", has_preprocessing=" << header.hasPreprocessing() << "\n";

    // 2. 读取预处理表（如果存在）
    preprocessor preprocessor;
    if (header.hasPreprocessing()) {
        if (!preprocessor.deserialize_table(src_file)) {
            std::cerr << "Error: failed to read preprocessing table\n";
            return false;
        }
    }

    // 3. LZW 解压
    BitReader bit_reader(src_file);
    LZWDecompressor decompressor(LZWDecompressOptions(9, header.max_code_width));

    // 创建临时文件来收集解压数据
    std::string temp_file = dst_path + ".tmp";
    std::ofstream temp_out(temp_file, std::ios::binary);

    if (!decompressor.decompressStream(bit_reader, temp_out)) {
        std::cerr << "Error: LZW decompression failed\n";
        temp_out.close();
        src_file.close();
        std::remove(temp_file.c_str());
        return false;
    }

    temp_out.close();
    src_file.close();

    // 4. 读取解压后的内容并进行预处理恢复
    std::ifstream temp_in(temp_file, std::ios::binary);
    std::string decompressed_content;
    decompressed_content.assign(std::istreambuf_iterator<char>(temp_in), std::istreambuf_iterator<char>());
    temp_in.close();
    std::remove(temp_file.c_str());

    if (header.hasPreprocessing()) {
        decompressed_content = preprocessor.restore(decompressed_content);
    }

    // 5. 写入最终结果
    std::ofstream dst_file(dst_path, std::ios::binary | std::ios::trunc);
    if (!dst_file) {
        std::cerr << "Error: cannot open destination file for writing\n";
        return false;
    }

    dst_file << decompressed_content;
    dst_file.close();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Decompression complete!\n";
    std::cout << "Output size: " << decompressed_content.length() << " bytes\n";
    std::cout << "Expected size: " << header.original_size << " bytes\n";
    std::cout << "Time taken: " << duration.count() << " ms\n";
    std::cout << "Dictionary entries: " << decompressor.getDictSize() << "\n";
    std::cout << "Codes read: " << decompressor.getCodesRead() << "\n";

    return decompressed_content.length() == header.original_size;
}

int main(int argc, char* argv[]) {
    std::cout << "LZW File Compressor v1.0\n";
    std::cout << "Author: @logarithm1110\n\n";

    ParsedArgs args;
    if (!parseArgs(argc, argv, args)) {
        return -1;
    }

    bool success = false;
    if (args.mode == "zip") {
        success = compressFile(args.src, args.dst);
    }
    else {
        success = decompressFile(args.src, args.dst);
    }

    return success ? 0 : -1;
}