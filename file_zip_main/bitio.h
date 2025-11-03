#ifndef BITIO_H
#define BITIO_H

#include <fstream>
#include <cstdint>
#include <vector>

// 位写入器：将可变位宽的代码写入到字节流
class BitWriter {
public:
    explicit BitWriter(std::ofstream& out);
    ~BitWriter();

    // 写入指定位宽的代码
    bool write(uint32_t code, int width);

    // 刷新缓冲区（将剩余位补0并写出）
    bool flush();

    // 获取已写入的位数
    uint64_t getBitsWritten() const { return bits_written_; }

private:
    std::ofstream& out_;
    uint32_t buffer_;       // 位缓冲区
    int buffer_bits_;       // 缓冲区中的有效位数
    uint64_t bits_written_; // 总写入位数
};

// 位读取器：从字节流读取可变位宽的代码
class BitReader {
public:
    explicit BitReader(std::ifstream& in);

    // 读取指定位宽的代码
    bool read(uint32_t& code, int width);

    // 检查是否还有数据可读
    bool hasMore() const;

    // 获取已读取的位数
    uint64_t getBitsRead() const { return bits_read_; }

private:
    std::ifstream& in_;
    uint32_t buffer_;       // 位缓冲区
    int buffer_bits_;       // 缓冲区中的有效位数
    uint64_t bits_read_;    // 总读取位数
    bool eof_reached_;      // 是否到达文件末尾

    // 填充缓冲区
    bool fillBuffer();
};

#endif