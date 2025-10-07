#include "jsonParser.h"

std::vector<std::string> jsonParser::getHeaders(const nlohmann::json& data)
{
    std::vector<std::string> headers;

    try {
        const auto& entries = data
            .value("log", nlohmann::json::object())
            .value("entries", nlohmann::json::array());

        if (!entries.empty()) {
            const auto& first_entry = entries[0];
            const auto& request_headers = first_entry
                .value("request", nlohmann::json::object())
                .value("headers", nlohmann::json::array());

            for (const auto& header : request_headers) {
                std::string header_str = header.value("name", "") + ": " +
                    header.value("value", "");
                if (!header_str.empty() && header_str != ": ") {
                    headers.push_back(header_str);
                }
            }
        }
    }
    catch (...) {
        // Exception handled by returning empty vector
    }

    return headers;
}

std::vector<std::string> jsonParser::getHeaders(const std::string& data)
{
    return getHeaders(nlohmann::json::parse(data));
}

std::string jsonParser::getURL(const nlohmann::json& data)
{
    std::string url;
    try {
        const auto& entries = data
            .value("log", nlohmann::json::object())
            .value("entries", nlohmann::json::array());
        if (!entries.empty())
        {
            url = entries[0].value("request", nlohmann::json::object())
                .value("url", "");
        }
    }
    catch (...) {
        // Exception handled by returning empty string
    }

    return url;
}

std::string jsonParser::getURL(const std::string& data)
{
    return getURL(nlohmann::json::parse(data));
}

std::string jsonParser::getMethod(const nlohmann::json& data)
{
    std::string method = "GET";
    try {
        const auto& entries = data
            .value("log", nlohmann::json::object())
            .value("entries", nlohmann::json::array());
        if (!entries.empty())
        {
            method = entries[0].value("request", nlohmann::json::object())
                .value("method", "GET");
        }
    }
    catch (...) {
        // Exception handled by returning default "GET"
    }

    return method;
}

std::string jsonParser::getMethod(const std::string& data)
{
    return getMethod(nlohmann::json::parse(data));
}

std::string jsonParser::getPostData(const nlohmann::json& data)
{
    std::string postData;
    try {
        const auto& entries = data
            .value("log", nlohmann::json::object())
            .value("entries", nlohmann::json::array());
        if (!entries.empty())
        {
            const auto& request = entries[0].value("request", nlohmann::json::object());

            const auto& postDataObj = request.value("postData", nlohmann::json::object());
            if (!postDataObj.is_null()) {
                postData = postDataObj.value("text", "");
            }

            if (postData.empty()) {
                postData = request.value("text", "");
            }
        }
    }
    catch (...) {
        // Exception handled by returning empty string
    }

    return postData;
}

std::string jsonParser::getPostData(const std::string& data)
{
    return getPostData(nlohmann::json::parse(data));
}