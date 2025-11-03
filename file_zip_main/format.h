#ifndef FORMAT_H
#define FORMAT_H

#include <cstdint>
#include <fstream>
#include <array>
#include <string>
#include <iostream>

// 压缩文件头定义与序列化/反序列化工具
// Magic: 4 bytes, e.g. "LZWC"
// Version: 1 byte
// Flags: 1 byte (bitflags for options e.g. preprocessing)
// Reserved: 2 bytes (对齐/未来扩展)
// OriginalSize: uint64_t (8 bytes)
// Extra: uint16_t max_code_width (2 bytes) 最大码宽（例如 12）

struct ArchiveHeader {
    std::array<char, 4> magic; // e.g. {'L','Z','W','C'}
    uint8_t version;          // 1
    uint8_t flags;            // bit flags: bit 0 = has_preprocessing
    uint16_t reserved;        // 保留对齐
    uint64_t original_size;   // 原始文件大小（字节）
    uint16_t max_code_width;  // 最大码宽（例如 12）

    // Flag 位定义
    static const uint8_t FLAG_HAS_PREPROCESSING = 0x01;

    ArchiveHeader() {
        magic = { 'L','Z','W','C' };
        version = 1;
        flags = 0;
        reserved = 0;
        original_size = 0;
        max_code_width = 12;
    }

    // 检查是否启用了预处理
    bool hasPreprocessing() const {
        return (flags & FLAG_HAS_PREPROCESSING) != 0;
    }

    // 设置预处理标志
    void setPreprocessing(bool enabled) {
        if (enabled) {
            flags |= FLAG_HAS_PREPROCESSING;
        }
        else {
            flags &= ~FLAG_HAS_PREPROCESSING;
        }
    }
};

// Helper: write a little-endian integer to stream
inline void write_le(std::ofstream& out, const uint16_t v) {
    uint8_t b0 = v & 0xFF;
    uint8_t b1 = (v >> 8) & 0xFF;
    out.put(static_cast<char>(b0));
    out.put(static_cast<char>(b1));
}

inline void write_le(std::ofstream& out, const uint64_t v) {
    for (int i = 0; i < 8; ++i) {
        out.put(static_cast<char>((v >> (8 * i)) & 0xFF));
    }
}

// Helper: read little-endian integer from stream
inline bool read_le(std::ifstream& in, uint16_t& out_v) {
    int b0 = in.get();
    if (b0 == EOF) return false;
    int b1 = in.get();
    if (b1 == EOF) return false;
    out_v = static_cast<uint16_t>(uint8_t(b0) | (uint16_t(uint8_t(b1)) << 8));
    return true;
}

inline bool read_le(std::ifstream& in, uint64_t& out_v) {
    out_v = 0;
    for (int i = 0; i < 8; ++i) {
        int b = in.get();
        if (b == EOF) return false;
        out_v |= (uint64_t(uint8_t(b)) << (8 * i));
    }
    return true;
}

// 写 header 到输出流（ofstream must be opened binary）
inline bool writeHeader(std::ofstream& out, const ArchiveHeader& h) {
    if (!out) return false;
    // magic 4 bytes
    out.write(h.magic.data(), 4);
    // version and flags
    out.put(static_cast<char>(h.version));
    out.put(static_cast<char>(h.flags));
    // reserved (2 bytes little-endian)
    write_le(out, h.reserved);
    // original_size (8 bytes little-endian)
    write_le(out, h.original_size);
    // max_code_width (2 bytes)
    write_le(out, h.max_code_width);
    return out.good();
}

// 读取 header（ifstream must be opened binary）
inline bool readHeader(std::ifstream& in, ArchiveHeader& h) {
    if (!in) return false;
    char mag[4];
    in.read(mag, 4);
    if (in.gcount() != 4) return false;
    for (int i = 0; i < 4; ++i) h.magic[i] = mag[i];
    int v = in.get();
    if (v == EOF) return false;
    h.version = static_cast<uint8_t>(v);
    v = in.get();
    if (v == EOF) return false;
    h.flags = static_cast<uint8_t>(v);
    if (!read_le(in, h.reserved)) return false;
    if (!read_le(in, h.original_size)) return false;
    if (!read_le(in, h.max_code_width)) return false;
    return true;
}

// 校验 magic 是否匹配
inline bool headerMagicOk(const ArchiveHeader& h) {
    return h.magic[0] == 'L' && h.magic[1] == 'Z' && h.magic[2] == 'W' && h.magic[3] == 'C';
}

// Convert magic to string for debugging
inline std::string magicToString(const ArchiveHeader& h) {
    return std::string(h.magic.data(), 4);
}

#endif 



//#ifndef FORMAT_H
//#define FORMAT_H
//
//#include<iostream>
//#include <fstream>
//#include <string>
//#include <cstdint>
//#include <array>
//
///*魔数标识：快速识别文件类型
//
//版本控制：支持格式演进
//
//小端序：兼容x86架构
//
//扩展性：保留字段和标志位为未来功能预留空间
//
//完整性检查：包含原始文件大小用于验证
//
//这个格式设计为LZW压缩算法提供了标准化的文件容器。*/
//
//struct ArchiveHeader {
//    std::array<char,4> magic; // 文件标识符 {'L','Z','W','C'}
//    uint8_t version;          // 格式版本号
//    uint8_t flags;            // 功能标志位 bit flags, 0 表示默认
//    uint16_t reserved;        // 保留字段，用于对齐和扩展
//    uint64_t original_size;   // 原始文件大小（8字节）
//    uint16_t max_code_width;  // 最大编码宽度（如12位）
//
//    ArchiveHeader() {
//        magic = { 'L','Z','W','C' };//设置魔数标识
//        version = 1;//版本1
//        flags = 0;//默认无特殊标志
//        reserved = 0;//保留位清零
//		original_size = 0;//初始原始大小为0
//		max_code_width = 12; // 默认最大编码宽度12位
//    }
//};//定义了压缩文件的头部信息，总共4+1+1+2+8+2=18字节？
//
//// 写入16位整数（小端序）
//inline void write_le(std::ofstream& out, const uint16_t v) {
//    uint8_t b0 = v & 0xFF;//低8位
//	uint8_t b1 = (v >> 8) & 0xFF;//高8位
//	out.put(static_cast<char>(b0));//先写低8位
//	out.put(static_cast<char>(b1));//再写高8位
//}
//
//// 写入64位整数（小端序）
//inline void write_le(std::ofstream& out, const uint64_t v) {
//    for (int i = 0; i < 8; ++i) {
//        // 从最低字节到最高字节依次写入
//        out.put(static_cast<char>((v >> (8 * i)) & 0xFF));
//    }
//}
//
//// 读取16位整数（小端序）
//inline bool read_le(std::ifstream& in, uint16_t& out_v) {
//    int b0 = in.get();//读取低字节
//    if (b0 == EOF) return false;
//	int b1 = in.get();//读取高字节
//    if (b1 == EOF) return false;
//	//组合成16位整数：低字节在前，高字节在后（<=8)
//    out_v = static_cast<uint16_t>(uint8_t(b0) | (uint16_t(uint8_t(b1)) << 8));
//    return true;
//}
//
//// 读取64位整数（小端序）
//inline bool read_le(std::ifstream& in, uint64_t& out_v) {
//    out_v = 0;
//    for (int i = 0; i < 8; ++i) {
//        int b = in.get(); //依次读取8个字节
//        if (b == EOF) return false;
//        // 将每个字节放到正确位置
//        out_v |= (uint64_t(uint8_t(b)) << (8 * i));
//    }
//    return true;
//}
//
//// 写 header 到输出流（ofstream must be opened binary）
//inline bool writeHeader(std::ofstream& out, const ArchiveHeader& h) {
//    if (!out) return false;
//
//    // 按顺序写入各个字段：
//	out.write(h.magic.data(), 4);//4字节魔数
//	out.put(static_cast<char>(h.version));//1字节版本号
//	out.put(static_cast<char>(h.flags));//1字节标志位
//	write_le(out, h.reserved);//2字节保留位
//	write_le(out, h.original_size);//8字节原始文件大小
//	write_le(out, h.max_code_width);//2字节最大编码宽度
//
//	return out.good();//检查写入是否成功
//}
//
//// 读取 header（ifstream must be opened binary）
//inline bool readHeader(std::ifstream& in, ArchiveHeader& h) {
//    if (!in) return false;
//
//    char mag[4];
//	in.read(mag, 4);//读取4字节魔数
//	if (in.gcount() != 4) return false;//检查是否成功读取4字节
//	for (int i = 0; i < 4; ++i) h.magic[i] = mag[i];//复制到结构体中
//
//	int v = in.get();//读取版本号
//    if (v == EOF) return false;
//    h.version = static_cast<uint8_t>(v);
//
//	v = in.get();//读取标志位
//    if (v == EOF) return false;
//    h.flags = static_cast<uint8_t>(v);
//
//	//读取剩余字段（小端序）
//    if (!read_le(in, h.reserved)) return false;
//    if (!read_le(in, h.original_size)) return false;
//    if (!read_le(in, h.max_code_width)) return false;
//    return true;
//}
//
////验证魔数是否正确
//inline bool headerMagicOk(const ArchiveHeader& h) {
//    return h.magic[0] == 'L' && h.magic[1] == 'Z' && h.magic[2] == 'W' && h.magic[3] == 'C';
//}
//
////将魔数字符数组转换为字符串（用于调试）
//inline std::string magicToString(const ArchiveHeader& h) {
//    return std::string(h.magic.data(), 4);
//}
//
//#endif