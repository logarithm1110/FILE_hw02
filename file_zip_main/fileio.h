//#ifndef FORMAT_H
//#define FORMAT_H
//
//#include <cstdint>
//#include <fstream>
//#include <array>
//#include <string>
//#include <iostream>
//
//// 压缩文件头定义与序列化/反序列化工具
//// Magic: 4 bytes, e.g. "LZWC"
//// Version: 1 byte
//// Flags: 1 byte (bitflags for options e.g. preprocessing)
//// Reserved: 2 bytes (对齐/未来扩展)
//// OriginalSize: uint64_t (8 bytes)
//// Extra: uint16_t initial_code_width (2 bytes) 可选字段 —— 这里写入 max_code_width（例如 12）
//
//struct ArchiveHeader {
//    std::array<char, 4> magic; // e.g. {'L','Z','W','C'}
//    uint8_t version;          // 1
//    uint8_t flags;            // bit flags: bit 0 = has_preprocessing
//    uint16_t reserved;        // 保留对齐
//    uint64_t original_size;   // 原始文件大小（字节）
//    uint16_t max_code_width;  // 最大码宽（例如 12）
//
//    // Flag 位定义
//    static const uint8_t FLAG_HAS_PREPROCESSING = 0x01;
//
//    ArchiveHeader() {
//        magic = { 'L','Z','W','C' };
//        version = 1;
//        flags = 0;
//        reserved = 0;
//        original_size = 0;
//        max_code_width = 12;
//    }
//
//    bool hasPreprocessing() const { return (flags & FLAG_HAS_PREPROCESSING) != 0; }
//    void setPreprocessing(bool enabled) {
//        if (enabled) {
//            flags |= FLAG_HAS_PREPROCESSING;
//        }
//        else {
//            flags &= ~FLAG_HAS_PREPROCESSING;
//        }
//    }
//};
//
//// Helper: write a little-endian integer to stream
//inline void write_le(std::ofstream& out, const uint16_t v) {
//    uint8_t b0 = v & 0xFF;
//    uint8_t b1 = (v >> 8) & 0xFF;
//    out.put(static_cast<char>(b0));
//    out.put(static_cast<char>(b1));
//}
//inline void write_le(std::ofstream& out, const uint64_t v) {
//    for (int i = 0; i < 8; ++i) {
//        out.put(static_cast<char>((v >> (8 * i)) & 0xFF));
//    }
//}
//
//// Helper: read little-endian integer from stream
//inline bool read_le(std::ifstream& in, uint16_t& out_v) {
//    int b0 = in.get();
//    if (b0 == EOF) return false;
//    int b1 = in.get();
//    if (b1 == EOF) return false;
//    out_v = static_cast<uint16_t>(uint8_t(b0) | (uint16_t(uint8_t(b1)) << 8));
//    return true;
//}
//inline bool read_le(std::ifstream& in, uint64_t& out_v) {
//    out_v = 0;
//    for (int i = 0; i < 8; ++i) {
//        int b = in.get();
//        if (b == EOF) return false;
//        out_v |= (uint64_t(uint8_t(b)) << (8 * i));
//    }
//    return true;
//}
//
//// 写 header 到输出流（ofstream must be opened binary）
//inline bool writeHeader(std::ofstream& out, const ArchiveHeader& h) {
//    if (!out) return false;
//    // magic 4 bytes
//    out.write(h.magic.data(), 4);
//    // version and flags
//    out.put(static_cast<char>(h.version));
//    out.put(static_cast<char>(h.flags));
//    // reserved (2 bytes little-endian)
//    write_le(out, h.reserved);
//    // original_size (8 bytes little-endian)
//    write_le(out, h.original_size);
//    // max_code_width (2 bytes)
//    write_le(out, h.max_code_width);
//    return out.good();
//}
//
//// 读取 header（ifstream must be opened binary）
//inline bool readHeader(std::ifstream& in, ArchiveHeader& h) {
//    if (!in) return false;
//    char mag[4];
//    in.read(mag, 4);
//    if (in.gcount() != 4) return false;
//    for (int i = 0; i < 4; ++i) h.magic[i] = mag[i];
//    int v = in.get();
//    if (v == EOF) return false;
//    h.version = static_cast<uint8_t>(v);
//    v = in.get();
//    if (v == EOF) return false;
//    h.flags = static_cast<uint8_t>(v);
//    if (!read_le(in, h.reserved)) return false;
//    if (!read_le(in, h.original_size)) return false;
//    if (!read_le(in, h.max_code_width)) return false;
//    return true;
//}
//
//// 校验 magic 是否匹配
//inline bool headerMagicOk(const ArchiveHeader& h) {
//    return h.magic[0] == 'L' && h.magic[1] == 'Z' && h.magic[2] == 'W' && h.magic[3] == 'C';
//}
//
//// Convert magic to string for debugging
//inline std::string magicToString(const ArchiveHeader& h) {
//    return std::string(h.magic.data(), 4);
//}
//
//#endif // FORMAT_H
//
//
////#ifndef FORMAT_H
////#define FORMAT_H
////
////#include <cstdint>
////#include <fstream>
////#include <array>
////#include <string>
////#include <iostream>
////
////// 压缩文件头定义与序列化/反序列化工具
////// Magic: 4 bytes, e.g. "LZWC"
////// Version: 1 byte
////// Flags: 1 byte (bitflags for options e.g. preprocessing)
////// Reserved: 2 bytes (对齐/未来扩展)
////// OriginalSize: uint64_t (8 bytes)
////// Extra: uint16_t initial_code_width (2 bytes) 可选字段 —— 这里写入 max_code_width（例如 12）
////
////struct ArchiveHeader {
////    std::array<char,4> magic; // e.g. {'L','Z','W','C'}
////    uint8_t version;          // 1
////    uint8_t flags;            // bit flags: bit 0 = has_preprocessing
////    uint16_t reserved;        // 保留对齐
////    uint64_t original_size;   // 原始文件大小（字节）
////    uint16_t max_code_width;  // 最大码宽（例如 12）
////    
////    // Flag 位定义
////    static const uint8_t FLAG_HAS_PREPROCESSING = 0x01;
////    
////    ArchiveHeader() {
////        magic = {'L'，'Z','W','C'};
////        version = 1;
////        flags = 0;
////        reserved = 0;
////        original_size = 0;
////        max_code_width = 12;
////    }
////    
////    bool hasPreprocessing() const { return (flags & FLAG_HAS_PREPROCESSING) != 0; }
////    void setPreprocessing(bool enabled) {
////        if (enabled) {
////            flags |= FLAG_HAS_PREPROCESSING;
////        } else {
////            flags &= ~FLAG_HAS_PREPROCESSING;
////        }
////    }
////};
////
////// Helper: write a little-endian integer to stream
////inline void write_le(std::ofstream &out, const uint16_t v) {
////    uint8_t b0 = v & 0xFF;
////    uint8_t b1 = (v >> 8) & 0xFF;
////    out.put(static_cast<char>(b0));
////    out.put(static_cast<char>(b1));
////}
////inline void write_le(std::ofstream &out, const uint64_t v) {
////    for (int i = 0; i < 8; ++i) {
////        out.put(static_cast<char>((v >> (8*i)) & 0xFF));
////    }
////}
////
////// Helper: read little-endian integer from stream
////inline bool read_le(std::ifstream &in, uint16_t &out_v) {
////    int b0 = in.get();
////    if (b0 == EOF) return false;
////    int b1 = in.get();
////    if (b1 == EOF) return false;
////    out_v = static_cast<uint16_t>(uint8_t(b0) | (uint16_t(uint8_t(b1)) << 8));
////    return true;
////}
////inline bool read_le(std::ifstream &in, uint64_t &out_v) {
////    out_v = 0;
////    for (int i = 0; i < 8; ++i) {
////        int b = in.get();
////        if (b == EOF) return false;
////        out_v |= (uint64_t(uint8_t(b)) << (8*i));
////    }
////    return true;
////}
////
////// 写 header 到输出流（ofstream must be opened binary）
////inline bool writeHeader(std::ofstream &out, const ArchiveHeader &h) {
////    if (!out) return false;
////    // magic 4 bytes
////    out.write(h.magic。data(), 4);
////    // version and flags
////    out.put(static_cast<char>(h.version));
////    out.put(static_cast<char>(h.flags));
////    // reserved (2 bytes little-endian)
////    write_le(out, h.reserved);
////    // original_size (8 bytes little-endian)
////    write_le(out, h.original_size);
////    // max_code_width (2 bytes)
////    write_le(out, h.max_code_width);
////    return out.good();
////}
////
////// 读取 header（ifstream must be opened binary）
////inline bool readHeader(std::ifstream &in, ArchiveHeader &h) {
////    if (!in) return false;
////    char mag[4];
////    in.read(mag, 4);
////    if (in.gcount() != 4) return false;
////    for (int i = 0; i < 4; ++i) h.magic[i] = mag[i];
////    int v = in.get();
////    if (v == EOF) return false;
////    h.version = static_cast<uint8_t>(v);
////    v = in.get();
////    if (v == EOF) return false;
////    h.flags = static_cast<uint8_t>(v);
////    if (!read_le(in, h.reserved)) return false;
////    if (!read_le(in, h.original_size)) return false;
////    if (!read_le(in, h.max_code_width)) return false;
////    return true;
////}
////
////// 校验 magic 是否匹配
////inline bool headerMagicOk(const ArchiveHeader &h) {
////    return h.magic[0]=='L' && h.magic[1]=='Z' && h.magic[2]=='W' && h.magic[3]=='C';
////}
////
////// Convert magic to string for debugging
////inline std::string magicToString(const ArchiveHeader &h) {
////    return std::string(h.magic.data(), 4);
////}
////
////#endif // FORMAT_H

#ifndef FILEIO_H
#define FILEIO_H

#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <iostream>

class BufferedFileReader {
public:
    BufferedFileReader() = default;
    ~BufferedFileReader();

    // 打开文件，返回是否成功
    bool open(const std::string& path, std::size_t buffer_size = 64 * 1024);

    // 从流中读取最多 size 字节到 buf，返回实际读取字节数（0 表示 EOF）
    std::size_t readChunk(char* buf, std::size_t size);

    // 获取文件总大小（如果无法获取返回 0）
    uint64_t fileSize() const;

    void close();

    bool isOpen() const { return in_.is_open(); }

private:
    std::ifstream in_;
    std::vector<char> buffer_;
    std::size_t buffer_size_ = 0;
    uint64_t file_size_ = 0;
};

class BufferedFileWriter {
public:
    BufferedFileWriter() = default;
    ~BufferedFileWriter();

    bool open(const std::string& path, std::size_t buffer_size = 64 * 1024);

    // 写入 size 字节，从 buf，返回是否成功（非 0 表示成功）
    bool writeChunk(const char* buf, std::size_t size);

    // flush 并关闭
    void close();

    bool isOpen() const { return out_.is_open(); }

private:
    std::ofstream out_;
    std::vector<char> buffer_;
    std::size_t buffer_size_ = 0;
};

#endif