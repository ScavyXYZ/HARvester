#include "request.h"

request::request()
{
    m_curl = curl_easy_init();

    if (m_curl)
    {
        m_response = new response;

        setCurlOPT(CURLOPT_WRITEFUNCTION, defaultCallbackHandle);
        setCurlOPT(CURLOPT_WRITEDATA, &m_response->body);

        setCurlOPT(CURLOPT_HEADERFUNCTION, defaultHeaderHandle);
        setCurlOPT(CURLOPT_HEADERDATA, &m_response->headers);
    }
    else
    {
        throw std::runtime_error("Curl init failure");
    }
}

request::request(const std::string& l_URL)
{
    m_curl = curl_easy_init();

    if (m_curl)
    {
        m_response = new response;

        setCurlOPT(CURLOPT_URL, l_URL.c_str());

        setCurlOPT(CURLOPT_WRITEFUNCTION, defaultCallbackHandle);
        setCurlOPT(CURLOPT_WRITEDATA, &m_response->body);

        setCurlOPT(CURLOPT_HEADERFUNCTION, defaultHeaderHandle);
        setCurlOPT(CURLOPT_HEADERDATA, &m_response->headers);
    }
    else
    {
        throw std::runtime_error("Curl init failure");
    }
}

request::request(std::ifstream& file)
{
    if (file.is_open())
    {
        m_curl = curl_easy_init();

        if (m_curl)
        {
            m_response = new response;

            nlohmann::json data;
            file >> data;

            setHeaders(jsonParser::getHeaders(data));
            setCurlOPT(CURLOPT_URL, jsonParser::getURL(data).c_str());

            std::string method = jsonParser::getMethod(data);
            if (method == "POST") {
                setCurlOPT(CURLOPT_POST, 1L);

                std::string postData = jsonParser::getPostData(data);
                if (!postData.empty()) {
                    setCurlOPT(CURLOPT_POSTFIELDS, postData.c_str());
                    setCurlOPT(CURLOPT_POSTFIELDSIZE, postData.length());
                }
            }

            setCurlOPT(CURLOPT_WRITEFUNCTION, defaultCallbackHandle);
            setCurlOPT(CURLOPT_WRITEDATA, &m_response->body);

            setCurlOPT(CURLOPT_HEADERFUNCTION, defaultHeaderHandle);
            setCurlOPT(CURLOPT_HEADERDATA, &m_response->headers);
        }
        else
        {
            throw std::runtime_error("Curl init failure");
        }
    }
    else
    {
        throw std::runtime_error("Can not open file");
    }
}

request::~request()
{
    for (auto& iter : m_options) {
        if (iter.first == CURLOPT_POSTFIELDS || iter.first == CURLOPT_COPYPOSTFIELDS) {
            if (auto* str_ptr = std::any_cast<std::string*>(&iter.second)) {
                delete* str_ptr;
            }
        }
    }
    m_options.clear();

    if (m_headers) {
        curl_slist_free_all(m_headers);
    }
    if (m_curl)
    {
        curl_easy_cleanup(m_curl);
    }
    if (m_response)
    {
        delete m_response;
    }
}

void request::send()
{
    if (m_headers)
    {
        setCurlOPT(CURLOPT_HTTPHEADER, m_headers);
    }
    setCurlOPT(CURLOPT_FOLLOWLOCATION, 1L);
    for (auto& iter : m_options)
    {
        setCurlOPT(iter.first, iter.second);
    }
    m_code = curl_easy_perform(m_curl);
}

response request::getResponse() const
{
    return *m_response;
}

std::string request::getError() const
{
    return curl_easy_strerror(getCurlCode());
}

bool request::isOK() const
{
    return getCurlCode() == CURLE_OK;
}

void request::setHeaders(const std::vector<std::string>& vec)
{
    if (m_headers) {
        curl_slist_free_all(m_headers);
        m_headers = nullptr;
    }

    for (auto& iter : vec)
    {
        m_headers = curl_slist_append(m_headers, iter.c_str());
    }
}

void request::addHeader(const std::string& header)
{
    m_headers = curl_slist_append(m_headers, header.c_str());
}

void request::addHeader(const std::vector<std::string>& vec)
{
    for (auto& iter : vec)
    {
        m_headers = curl_slist_append(m_headers, iter.c_str());
    }
}

void request::setURL(const std::string& URL)
{
    setCurlOPT(CURLOPT_URL, URL.c_str());
}

void request::setPostFields(const std::string& fields)
{
    setopt(CURLOPT_POST, 1L);
    setopt(CURLOPT_POSTFIELDS, fields);
    setopt(CURLOPT_POSTFIELDSIZE, fields.length());
}

CURLcode request::getCurlCode() const
{
    return m_code;
}

CURL* request::getCurl() const
{
    return m_curl;
}

size_t request::defaultCallbackHandle(void* ptr, size_t size, size_t nmemb, std::string* data)
{
    data->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

size_t request::defaultHeaderHandle(char* buffer, size_t size, size_t nitems, void* userdata)
{
    std::string headerLine(buffer, size * nitems);

    auto* headers = static_cast<std::unordered_map<std::string, std::string>*>(userdata);

    size_t colonPos = headerLine.find(": ");
    if (colonPos != std::string::npos) {
        std::string key = headerLine.substr(0, colonPos);
        std::string value = headerLine.substr(colonPos + 2);

        if (!value.empty() && value.back() == '\r') {
            value.pop_back();
        }
        if (!value.empty() && value.back() == '\n') {
            value.pop_back();
        }

        (*headers)[key] = value;
    }

    return size * nitems;
}

//response class
std::unordered_map<std::string, std::string> response::getHeaders() const
{
    return headers;
}

std::string response::getBody() const
{
    auto it = headers.find("content-encoding");
    if (it != headers.end()) {
        return decompressor::decompress(body, it->second);
    }
    return body;
}

std::ostream& operator<<(std::ostream& os, const std::unordered_map<std::string, std::string>& map)
{
    for (const auto& iter : map)
    {
        os << iter.first << ": " << iter.second << std::endl;
    }
    return os;
}

