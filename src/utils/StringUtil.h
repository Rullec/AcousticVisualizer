#ifndef STRING_UTIL_H
#define STRING_UTIL_H
#pragma once
#include <memory>
#include <string>
#include <vector>

class cStringUtil
{
public:
    static std::string
    ConcatenateString(const std::vector<std::string>::const_iterator &begin,
                      const std::vector<std::string>::const_iterator &end,
                      std::string delimiter);
    static std::string
    ConcatenateString(const std::vector<std::string> &str_vec,
                      std::string delimiter);
    static std::vector<std::string> SplitString(const std::string &raw_string,
                                                const std::string &delimiter);
    static void RemoveEmptyLine(std::vector<std::string> &lines);
    static void RemoveCommentLine(std::vector<std::string> &lines,
                                  std::string comment_delimeter = "#");
    static std::string Strip(std::string line);
    template <typename... Args>
    static std::string string_format(const std::string &format, Args... args)
    {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) +
                     1; // Extra space for '\0'
        if (size_s <= 0)
        {
            // throw std::runtime_error("Error during formatting.");
            return "";
        }
        auto size = static_cast<size_t>(size_s);
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size -
                                          1); // We don't want the '\0' inside
    }
};
#endif