#pragma once

#include <fstream>
#include <vector>
#include <boost/bimap.hpp>

static std::string log_type = "";

#ifdef DEBUG
#define LOG(...) \
    do { \
        if (!log_type.empty()) { \
            std::cout << "[" << log_type << "] " << __VA_ARGS__ << std::endl; \
        } \
    } while (0)
#define LOG_TYPE(type) \
    do { \
        log_type = (type); \
        std::cout << "======================================== " + log_type + " ========================================" << std::endl; \
    } while (0)
#else
#define LOG(...)
#define LOG_TYPE(type)
#endif

typedef unsigned char data_t;
typedef unsigned long long len_t;

class LZWCode {
private:
    len_t _data;
    data_t data[8];
public:
    LZWCode(len_t data);
    LZWCode(data_t* data, data_t size);
    len_t toLen();
    data_t* toData(data_t size);
};

class LZWCoder {
private:
    boost::bimap<std::vector<data_t>, len_t> table;
    std::vector<LZWCode> code_buffer;
    std::vector<data_t> char_buffer;
    std::vector<data_t> data_buffer;
    len_t cur_code, old_code;
    len_t code_size, code_len;
private:
    void reset();
    void updateCodeMeta();
    void readCode(bool lastread);
    void readChar(bool firstread);
    std::tuple<data_t*, len_t> codeData();
    std::tuple<data_t*, len_t> charData();
public:
    LZWCoder();
    std::tuple<data_t*, len_t> encode(const data_t* src, const len_t len);
    void encode(const std::string& dst, const std::string& src);
    std::tuple<data_t*, len_t> decode(const data_t* src, const len_t len);
    void decode(const std::string& dst, const std::string& src);
};