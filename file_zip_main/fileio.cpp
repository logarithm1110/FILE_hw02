#include "fileio.h"
#include <sys/stat.h>

BufferedFileReader::~BufferedFileReader() {
    close();
}

bool BufferedFileReader::open(const std::string& path, std::size_t buffer_size) {
    close();
    in_.open(path, std::ios::in | std::ios::binary);
    if (!in_) return false;
    buffer_size_ = buffer_size;
    buffer_.resize(buffer_size_ ? buffer_size_ : 1);
    // 尝试求文件大小（seek）
    in_.seekg(0, std::ios::end);
    std::streamoff endpos = in_.tellg();
    if (endpos >= 0) {
        file_size_ = static_cast<uint64_t>(endpos);
    }
    else {
        file_size_ = 0;
    }
    in_.seekg(0, std::ios::beg);
    return true;
}

std::size_t BufferedFileReader::readChunk(char* buf, std::size_t size) {
    if (!in_) return 0;
    in_.read(buf, static_cast<std::streamsize>(size));
    std::streamsize got = in_.gcount();
    if (got <= 0) return 0;
    return static_cast<std::size_t>(got);
}

uint64_t BufferedFileReader::fileSize() const {
    return file_size_;
}

void BufferedFileReader::close() {
    if (in_.is_open()) {
        in_.close();
    }
    buffer_.clear();
    buffer_.shrink_to_fit();
}

BufferedFileWriter::~BufferedFileWriter() {
    close();
}

bool BufferedFileWriter::open(const std::string& path, std::size_t buffer_size) {
    close();
    out_.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out_) return false;
    buffer_size_ = buffer_size;
    buffer_.resize(buffer_size_ ? buffer_size_ : 1);
    return true;
}

bool BufferedFileWriter::writeChunk(const char* buf, std::size_t size) {
    if (!out_) return false;
    out_.write(buf, static_cast<std::streamsize>(size));
    return out_.good();
}

void BufferedFileWriter::close() {
    if (out_.is_open()) {
        out_.flush();
        out_.close();
    }
    buffer_.clear();
    buffer_.shrink_to_fit();
}