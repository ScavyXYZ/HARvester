#include <iostream>
#include <fstream>
#include "request.h"

int main()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    //URL example
    request *r = new request("https://httpbingo.org/get");

    //you can also specify a vector as an argument
    r->addHeader("User-Agent: Harvester");
    r->setHeaders({"User-Agent: Harvester"});

    r->send();
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

    curl_global_cleanup();
    return 0;
}
