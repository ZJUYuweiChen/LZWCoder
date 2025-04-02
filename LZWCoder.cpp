#include <iostream>

#include "LZWCoder.hpp"

inline std::string to_string(data_t data, bool raw) {
    return raw ? std::to_string(data) : std::string(1, static_cast<char>(data));
}

inline std::string to_string(len_t len) {
    return std::to_string(len);
}

inline std::string to_string(std::vector<data_t> vector, bool raw) {
    std::string result = "";
    for (data_t c : vector) {
        result += to_string(c, raw);
        if (raw) result += " ";
    }
    return result;
}

inline std::string to_string(std::vector<LZWCode> vector) {
    std::string result = "";
    for (LZWCode c : vector) {
        result += to_string(c.toLen()) + " ";
    }
    return result;
}

std::tuple<data_t*, len_t> readFile(std::string filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return {nullptr, 0};
    std::streamsize len = file.tellg();
    data_t* data = new data_t[len];
    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(data), len)) return {nullptr, 0};
    file.close();
    return {data, len};
}

bool writeFile(std::string filename, data_t* data, len_t len) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;
    if (!file.write(reinterpret_cast<const char*>(data), len)) return false;
    file.close();
    return true;
}

LZWCode::LZWCode(len_t data) : _data(0) {
    this->_data = data;
    for (data_t i = 0; i < 8; i++) {
        this->data[i] = static_cast<data_t>((this->_data >> (56 - i * 8)) & 0xFF);
    }
}

LZWCode::LZWCode(data_t* data, data_t size) : _data(0) {
    for (len_t i = 0; i < 8; i++) {
        if (i < size) this->data[8 - size + i] = data[i];
        else this->data[8 - size + i] = 0;
    }
    for (len_t i = 0; i < 8; i++) {
        _data += static_cast<len_t>(this->data[i]) << (56 - i * 8);
    }
}

len_t LZWCode::toLen() {
    return _data;
}

data_t* LZWCode::toData(data_t size) {
    return data + 8 - size;
}

LZWCoder::LZWCoder() : table(), code_buffer(), char_buffer(), data_buffer(), cur_code(0), old_code(0), code_size(0), code_len(0) {
}

void LZWCoder::reset() {
    table.clear();
    code_buffer.clear();
    char_buffer.clear();
    data_buffer.clear();
    cur_code = 0;
    old_code = 0;
    code_size = 0;
    code_len = 0;
    for (len_t i = 0; i < 256; i++) {
        table.insert({{static_cast<data_t>(i)}, i});
    }
}

void LZWCoder::readCode(bool lastread) {
    if (lastread) {
        code_buffer.emplace_back(table.left.find(data_buffer)->second);
        data_buffer.clear();
    } else {
        table.insert({data_buffer, table.size()});
        data_t last = data_buffer.back();
        data_buffer.pop_back();
        code_buffer.emplace_back(table.left.find(data_buffer)->second);
        data_buffer.clear();
        data_buffer.push_back(last);
    }
}

void LZWCoder::readChar(bool firstread) {
    cur_code = 0;
    for (len_t i = 0; i < code_size; i++) {
        cur_code += static_cast<len_t>(data_buffer.back()) << (i * 8);
        data_buffer.pop_back();
    }
    LOG("       oldc: " + to_string(old_code));
    LOG("       curc: " + to_string(cur_code));
    if (firstread) {
        std::vector<data_t> cur_str = table.right.find(cur_code)->second;
        char_buffer.insert(char_buffer.end(), cur_str.begin(), cur_str.end());
        old_code = cur_code;
    } else {
        if (cur_code < table.size()) {
            std::vector<data_t> cur_str = table.right.find(cur_code)->second;
            char_buffer.insert(char_buffer.end(), cur_str.begin(), cur_str.end());
            std::vector<data_t> old_str = table.right.find(old_code)->second;
            old_str.emplace_back(cur_str[0]);
            table.insert({old_str, table.size()});
            old_code = cur_code;
        } else {
            std::vector<data_t> str = table.right.find(old_code)->second;
            str.emplace_back(str[0]);
            char_buffer.insert(char_buffer.end(), str.begin(), str.end());
            table.insert({str, table.size()});
            old_code = cur_code;
        }
    }
}

void LZWCoder::updateCodeMeta() {
    code_size = 0;
    code_len = code_buffer.size();
    len_t table_size = table.size();
    if (table_size == 0) {
        code_size = 1;
    }
    while (table_size > 0) {
        table_size >>= 8;
        code_size++;
    }
}

std::tuple<data_t*, len_t> LZWCoder::codeData() {
    updateCodeMeta();
    len_t data_len = code_len * code_size + 1;
    data_t* data = new data_t[data_len];
    data[0] = code_size;
    for (len_t i = 0; i < code_len; i++) {
        data_t* code_data = code_buffer[i].toData(code_size);
        for (len_t j = 0; j < code_size; j++) {
            data[i * code_size + j + 1] = code_data[j];
        }
    }
    return {data, data_len};
}

std::tuple<data_t*, len_t> LZWCoder::charData() {
    len_t data_len = char_buffer.size();
    data_t* data = new data_t[data_len];
    std::memcpy(data, char_buffer.data(), data_len);
    return {data, data_len};
}

std::tuple<data_t*, len_t> LZWCoder::encode(const data_t* src, const len_t len) {
    LOG("   resetting...");
    reset();
    LOG("   emplacing...");
    for (len_t i = 0; i < len; i++) {
        data_buffer.emplace_back(src[i]);
        LOG("       data: " + to_string(data_buffer, true));
        if (table.left.find(data_buffer) == table.left.end()) {
            readCode(false);
        }
        LOG("       code: " + to_string(code_buffer));
    }
    if (!data_buffer.empty()) {
        readCode(true);
    }
    LOG("       code: " + to_string(code_buffer));
    return codeData();
}

std::tuple<data_t*, len_t> LZWCoder::decode(const data_t* src, const len_t len) {
    LOG("   resetting...");
    reset();
    code_size = src[0];
    code_len = (len - 1) / code_size;
    LOG("       code_size: " + to_string(code_size));
    LOG("       code_len : " + to_string(code_len));
    LOG("   emplacing...");
    for (len_t i = 0; i < code_len; i++) {
        for (data_t j = 0; j < code_size; j++) {
            data_buffer.emplace_back(src[i * code_size + j + 1]);
        }
        readChar(i == 0);
        LOG("       char: " + to_string(char_buffer, false));
    }
    return charData();
}

void LZWCoder::encode(const std::string& dst, const std::string& src) {
    LOG_TYPE("ENCODER");
    LOG("SRC: " + src);
    LOG("DST: " + dst);
    LOG("Reading...");
    auto [src_data, src_len] = readFile(src);
    if (!src_data) throw std::runtime_error("Can't read file.");
    LOG("Encoding...");
    auto [dst_data, dst_len] = encode(src_data, src_len);
    LOG("Writing...");
    if (!writeFile(dst, dst_data, dst_len)) throw std::runtime_error("Can't write file.");
    LOG("Deleting...");
    delete[] src_data;
    delete[] dst_data;
    LOG("Encoded!");
}

void LZWCoder::decode(const std::string& dst, const std::string& src) {
    LOG_TYPE("DECODER");
    LOG("SRC: " + src);
    LOG("DST: " + dst);
    LOG("Reading...");
    auto [src_data, src_len] = readFile(src);
    if (!src_data) throw std::runtime_error("Can't read file.");
    LOG("Decoding...");
    auto [dst_data, dst_len] = decode(src_data, src_len);
    LOG("Writing...");
    if (!writeFile(dst, dst_data, dst_len)) throw std::runtime_error("Can't write file.");
    LOG("Deleting...");
    delete[] src_data;
    delete[] dst_data;
    LOG("Decoded!");
}