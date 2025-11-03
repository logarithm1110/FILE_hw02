#include "lzw_compress.h"
#include <iostream>
#include<sstream>

LZWCompressor::LZWCompressor(const LZWCompressOptions& options)
    : options_(options), next_code_(FIRST_CODE),
    current_code_width_(options.initial_code_width),
    input_size_(0), dict_size_(0), codes_written_(0) {
    initDictionary();
}

void LZWCompressor::initDictionary() {
    dictionary_.clear();

    // 添加所有单字节字符 (0-255)
    for (int i = 0; i < 256;++i) {
        std::string s(1, static_cast<char>(i));
        dictionary_[s] = static_cast<uint32_t>(i);
    }

    next_code_ = FIRST_CODE;
    current_code_width_ = options_.initial_code_width;
    dict_size_ = dictionary_.size();
}

void LZWCompressor::clearDictionary() {
    initDictionary();
}

bool LZWCompressor::shouldIncreaseCodeWidth() const {
    return next_code_ >= (1U << current_code_width_) &&
        current_code_width_ < options_.max_code_width;
}

bool LZWCompressor::isDictionaryFull() const {
    return next_code_ >= (1U << options_.max_code_width);
}

bool LZWCompressor::writeCode(BitWriter& out, uint32_t code) {
    codes_written_++;
    return out.write(code, current_code_width_);
}

bool LZWCompressor::compressStream(std::ifstream& in, BitWriter& out) {
    if (!in.good()) return false;

    initDictionary();
    input_size_ = 0;
    codes_written_ = 0;

    std::string current;
    int ch;

    while ((ch = in.get()) != EOF) {
        input_size_++;
        char byte = static_cast<char>(ch);
        std::string next = current + byte;

        if (dictionary_.find(next) != dictionary_.end()) {
            // 在字典中找到，继续读取
            current = next;
        }
        else {
            // 没找到，输出当前序列的代码
            if (!current.empty()) {
                auto it = dictionary_.find(current);
                if (it == dictionary_.end()) {
                    std::cerr << "Error: sequence not in dictionary: " << current << std::endl;
                    return false;
                }

                if (!writeCode(out, it->second)) {
                    return false;
                }
            }

            // 将新序列添加到字典（如果还有空间）
            if (!isDictionaryFull()) {
                dictionary_[next] = next_code_++;
                dict_size_++;

                // 检查是否需要增加码宽
                if (shouldIncreaseCodeWidth()) {
                    current_code_width_++;
                }
            }
            //else if (options_.use_clear_code) {
            //    // 字典满了，发送清空代码并重新初始化
            //    if (!writeCode(out, CLEAR_CODE)) {
            //        return false;
            //    }
            //    clearDictionary();
            //}

            // 开始新的序列
            current = std::string(1, byte);
        }
    }

    // 输出最后的序列
    if (!current.empty()) {
        auto it = dictionary_.find(current);
        if (it == dictionary_.end()) {
            std::cerr << "Error: final sequence not in dictionary" << std::endl;
            return false;
        }

        if (!writeCode(out, it->second)) {
            return false;
        }
    }

    // 写入EOF代码
    if (!writeCode(out, EOF_CODE)) {
        return false;
    }

    return true;
}

bool LZWCompressor::compressString(const std::string& input, BitWriter& out) {
    std::istringstream iss(input);
    std::ifstream dummy;

    // 将字符串内容复制到临时流
    std::string temp_filename = "/tmp/lzw_temp_" + std::to_string(reinterpret_cast<uintptr_t>(this));
    std::ofstream temp_out(temp_filename, std::ios::binary);
    if (!temp_out) return false;

    temp_out << input;
    temp_out.close();

    std::ifstream temp_in(temp_filename, std::ios::binary);
    if (!temp_in) return false;

    bool result = compressStream(temp_in, out);
    temp_in.close();

    // 清理临时文件
    std::remove(temp_filename.c_str());

    return result;
}