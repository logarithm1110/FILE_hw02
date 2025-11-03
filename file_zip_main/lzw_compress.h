#ifndef LZW_COMPRESS_H
#define LZW_COMPRESS_H

#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include "bitio.h"

// LZW 压缩器选项
struct LZWCompressOptions {
    int initial_code_width = 9;    // 初始码宽
    int max_code_width = 12;       // 最大码宽
    bool use_clear_code = true;    // 是否使用清空代码

    LZWCompressOptions() = default;
    LZWCompressOptions(int init_width, int max_width)
        : initial_code_width(init_width), max_code_width(max_width) {
    }
};

// LZW 压缩器
class LZWCompressor {
public:
    explicit LZWCompressor(const LZWCompressOptions& options = LZWCompressOptions());

    // 压缩数据流
    bool compressStream(std::ifstream& in, BitWriter& out);

    // 压缩字符串（用于测试）
    bool compressString(const std::string& input, BitWriter& out);

    // 获取统计信息
    size_t getInputSize() const { return input_size_; }
    size_t getDictSize() const { return dict_size_; }
    size_t getCodesWritten() const { return codes_written_; }

private:
    LZWCompressOptions options_;
    std::unordered_map<std::string,uint32_t> dictionary_;
    uint32_t next_code_;
    int current_code_width_;
    size_t input_size_;
    size_t dict_size_;
    size_t codes_written_;

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

    // 写入代码
    bool writeCode(BitWriter& out, uint32_t code);
};

#endif 