#ifndef LZW_DECOMPRESS_H
#define LZW_DECOMPRESS_H

#include <string>
#include <vector>
#include <fstream>
#include "bitio.h"

// LZW 解压器选项
struct LZWDecompressOptions {
    int initial_code_width = 9;
    int max_code_width = 12;
    bool use_clear_code = true;

    LZWDecompressOptions() = default;
    LZWDecompressOptions(int init_width, int max_width)
        : initial_code_width(init_width), max_code_width(max_width) {}
};

// LZW 解压器
class LZWDecompressor {
public:
    explicit LZWDecompressor(const LZWDecompressOptions& options = LZWDecompressOptions());

    // 解压数据流
    bool decompressStream(BitReader& in, std::ofstream& out);

    // 解压到字符串（用于测试）
    bool decompressToString(BitReader& in, std::string& output);

    // 获取统计信息
    size_t getOutputSize() const { return output_size_; }
    size_t getDictSize() const { return dict_size_; }
    size_t getCodesRead() const { return codes_read_; }

private:
    LZWDecompressOptions options_;
    std::vector<std::string> dictionary_;
    uint32_t next_code_;
    int current_code_width_;
    size_t output_size_;
    size_t dict_size_;
    size_t codes_read_;

    // 特殊代码
    static const uint32_t CLEAR_CODE = 256;
    static const uint32_t EOF_CODE = 257;
    static const uint32_t FIRST_CODE = 258;

    // 初始化字典
    void initDictionary();

    // 清空字典
    void clearDictionary();

    // 检查是否需要增加码宽
    bool shouldIncreaseCodeWidth() const;

    // 检查字典是否已满
    bool isDictionaryFull() const;

    // 读取代码
    bool readCode(BitReader& in, uint32_t& code);

    // 获取字典条目
    std::string getDictEntry(uint32_t code, const std::string& prev_string) const;
};

#endif 