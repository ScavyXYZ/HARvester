#include <iostream>
#include <fstream>
#include "request.h"

int main()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    request *r = new request;

    r->setURL("https://httpbingo.org/get"); // OR r->setopt(CURLOPT_URL, "https://httpbingo.org/get");
    r->addHeader("User-Agent: Harvester");
    r->send();
    std::cout << "Basic: ";
    if (r->isOK())
    {
        std::cout << "Success" << std::endl;
        std::cout << "Response Body:" << std::endl;
        std::cout << r->getResponse().getBody() << std::endl;
        std::cout << "Response Headers:" << std::endl;
        std::cout << r->getResponse().getHeaders();
    }
    else
    {
        std::cout << "ERROR: " << r->getError();
    }

    delete r;
    r = nullptr;

    curl_global_cleanup();
    return 0;
}
