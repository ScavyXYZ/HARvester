#pragma once
#include "json.hpp"
#include <string>
#include <vector>

class jsonParser
{
public:
    static std::vector<std::string> getHeaders(const nlohmann::json& data);
    static std::vector<std::string> getHeaders(const std::string& data);

    static std::string getURL(const nlohmann::json& data);
    static std::string getURL(const std::string& data);

    static std::string getMethod(const nlohmann::json& data);
    static std::string getMethod(const std::string& data);

    static std::string getPostData(const nlohmann::json& data);
    static std::string getPostData(const std::string& data);
};