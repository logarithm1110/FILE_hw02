#include "lzw_decompress.h"
#include <iostream>
#include<sstream>

LZWDecompressor::LZWDecompressor(const LZWDecompressOptions& options)
    : options_(options), next_code_(FIRST_CODE),
    current_code_width_(options.initial_code_width),
    output_size_(0), dict_size_(0), codes_read_(0) {
    initDictionary();
}

void LZWDecompressor::initDictionary() {
    dictionary_.clear();
    dictionary_.resize(1 << options_.max_code_width);

    // 添加所有单字节字符 (0-255)
    for (int i = 0; i < 256; ++i) {
        dictionary_[i] = std::string(1, static_cast<char>(i));
    }

    next_code_ = FIRST_CODE;
    current_code_width_ = options_.initial_code_width;
    dict_size_ = 256;
}

void LZWDecompressor::clearDictionary() {
    initDictionary();
}

bool LZWDecompressor::shouldIncreaseCodeWidth() const {
    return next_code_ >= (1U << current_code_width_) &&
        current_code_width_ < options_.max_code_width;
}

bool LZWDecompressor::isDictionaryFull() const {
    return next_code_ >= (1U << options_.max_code_width);
}

bool LZWDecompressor::readCode(BitReader& in, uint32_t& code) {
    codes_read_++;
    return in.read(code, current_code_width_);
}

std::string LZWDecompressor::getDictEntry(uint32_t code, const std::string& prev_string) const {
    if (code < next_code_) {
        return dictionary_[code];
    }
    else if (code == next_code_ && !prev_string.empty()) {
        // 处理 KwKwK 模式
        return prev_string + prev_string[0];
    }
    else {
        // 无效代码
        return "";
    }
}

bool LZWDecompressor::decompressStream(BitReader& in, std::ofstream& out) {
    if (!out.good()) return false;

    initDictionary();
    output_size_ = 0;
    codes_read_ = 0;

    uint32_t code;
    std::string prev_string;

    while (readCode(in, code)) {
        if (code == EOF_CODE) {
            // 到达文件末尾
            break;
        }

        if (code == CLEAR_CODE && options_.use_clear_code) {
            // 清空字典
            clearDictionary();
            prev_string.clear();
            continue;
        }

        // 获取当前代码对应的字符串
        std::string current_string = getDictEntry(code, prev_string);
        if (current_string.empty()) {
            std::cerr << "Error: invalid code " << code << std::endl;
            return false;
        }

        // 输出当前字符串
        out.write(current_string.c_str(), current_string.length());
        if (!out.good()) return false;
        output_size_ += current_string.length();

        // 如果不是第一个代码，添加新条目到字典
        if (!prev_string.empty() && !isDictionaryFull()) {
            std::string new_entry = prev_string + current_string[0];
            dictionary_[next_code_++] = new_entry;
            dict_size_++;

            // 检查是否需要增加码宽
            if (shouldIncreaseCodeWidth()) {
                current_code_width_++;
            }
        }

        prev_string = current_string;
    }

    return true;
}

bool LZWDecompressor::decompressToString(BitReader& in, std::string& output) {
    output.clear();

    std::string temp_filename = "/tmp/lzw_decomp_" + std::to_string(reinterpret_cast<uintptr_t>(this));
    std::ofstream temp_out(temp_filename, std::ios::binary);
    if (!temp_out) return false;

    bool result = decompressStream(in, temp_out);
    temp_out.close();

    if (result) {
        std::ifstream temp_in(temp_filename, std::ios::binary);
        if (temp_in) {
            std::ostringstream oss;
            oss << temp_in.rdbuf();
            output = oss.str();
            temp_in.close();
        }
        else {
            result = false;
        }
    }

    // 清理临时文件
    std::remove(temp_filename.c_str());

    return result;
}