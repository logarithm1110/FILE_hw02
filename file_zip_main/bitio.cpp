#include "bitio.h"
#include <iostream>

BitWriter::BitWriter(std::ofstream& out)
    : out_(out), buffer_(0), buffer_bits_(0), bits_written_(0) {
}

BitWriter::~BitWriter() {
    flush();
}

bool BitWriter::write(uint32_t code, int width) {
    if (width <= 0 || width > 32) return false;
    if (!out_.good()) return false;

    bits_written_ += width;

    // 将代码添加到缓冲区
    buffer_ |= (code << buffer_bits_);
    buffer_bits_ += width;

    // 当缓冲区有8位或更多时，写出完整的字节
    while (buffer_bits_ >= 8) {
        uint8_t byte = static_cast<uint8_t>(buffer_ & 0xFF);
        out_.put(static_cast<char>(byte));
        if (!out_.good()) return false;

        buffer_ >>= 8;
        buffer_bits_ -= 8;
    }

    return true;
}

bool BitWriter::flush() {
    if (!out_.good()) return false;

    // 如果缓冲区还有剩余位，补0并写出
    if (buffer_bits_ > 0) {
        uint8_t byte = static_cast<uint8_t>(buffer_ & 0xFF);
        out_.put(static_cast<char>(byte));
        buffer_ = 0;
        buffer_bits_ = 0;
    }

    out_.flush();
    return out_.good();
}

BitReader::BitReader(std::ifstream& in)
    : in_(in), buffer_(0), buffer_bits_(0), bits_read_(0), eof_reached_(false) {
}

bool BitReader::read(uint32_t& code, int width) {
    if (width <= 0 || width > 32) return false;

    code = 0;

    // 确保缓冲区有足够的位
    while (buffer_bits_ < width && !eof_reached_) {
        if (!fillBuffer()) {
            eof_reached_ = true;
            break;
        }
    }

    if (buffer_bits_ < width) {
        // 没有足够的位可读
        return false;
    }

    // 从缓冲区提取指定位数
    uint32_t mask = (1U << width) - 1;
    code = buffer_ & mask;
    buffer_ >>= width;
    buffer_bits_ -= width;
    bits_read_ += width;

    return true;
}

bool BitReader::hasMore() const {
    return buffer_bits_ > 0 || (!eof_reached_ && in_.good());
}

bool BitReader::fillBuffer() {
    if (!in_.good()) return false;

    int byte = in_.get();
    if (byte == EOF) {
        return false;
    }

    // 将新字节添加到缓冲区的高位
    buffer_ |= (static_cast<uint32_t>(static_cast<uint8_t>(byte)) << buffer_bits_);
    buffer_bits_ += 8;

    return true;
}