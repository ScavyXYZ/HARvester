#pragma once
#include <curl/curl.h>
#include <string>
#include <vector>
#include <any>
#include <unordered_map>
#include <fstream>
#include "decompressor.h"
#include "jsonParser.h"

std::ostream& operator<<(std::ostream& os, const std::unordered_map<std::string, std::string>& map);

struct response
{
public:
	std::unordered_map<std::string, std::string> getHeaders() const;
	std::string getBody() const;
private:
	std::unordered_map<std::string, std::string> headers;
	std::string body;
	friend class request;
};

class request
{
public:
	request();
	request(const std::string& URL);
	request(std::ifstream& file);
	~request();

	template<typename T>
	void setopt(const CURLoption& option, const T& parameter);

	void send();

	CURLcode    getCurlCode()        const;
	CURL* getCurl()            const;

	response    getResponse()        const;
	std::string getError()           const;
	bool        isOK()               const;

	template<size_t N>
	void setHeaders(const std::string(&headers)[N]);
	void setHeaders(const std::vector<std::string>& vec);

	void addHeader(const std::string& header);
	void addHeader(const std::vector<std::string>& vec);

	void setURL(const std::string& URL);
	void setPostFields(const std::string& fields);

private:
	CURLcode m_code = CURLE_OK;

	CURL* m_curl = nullptr;
	curl_slist* m_headers = nullptr;

	std::vector<std::pair<CURLoption, std::any>> m_options;

	template<typename T>
	void setCurlOPT(const CURLoption& option, const T& parameter);

	response* m_response = nullptr;
	static size_t defaultCallbackHandle(void* ptr, size_t size, size_t nmemb, std::string* data);
	static size_t defaultHeaderHandle(char* buffer, size_t size, size_t nitems, void* userdata);
};

//Template functions

template<size_t N>
inline void request::setHeaders(const std::string(&l_headers)[N])
{
	if (m_headers) {
		curl_slist_free_all(m_headers);
		m_headers = nullptr;
	}
	for (int i = 0; i < N; i++)
	{
		m_headers = curl_slist_append(m_headers, l_headers[i].c_str());
	}
}

template<typename T>
inline void request::setCurlOPT(const CURLoption& option, const T& parameter)
{
	curl_easy_setopt(m_curl, option, parameter);
}

template<typename T>
inline void request::setopt(const CURLoption& option, const T& parameter)
{
	m_options.push_back(std::make_pair(option, parameter));
}