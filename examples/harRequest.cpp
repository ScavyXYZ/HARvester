#include <iostream>
#include <fstream>
#include "request.h"

int main()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // HAR file example
    //open file
    std::ifstream file("request.har");
    if (!file) {
        std::cout << "ERROR: Cannot open request.har" << std::endl;
        return 1;
    }
    //creating request obj
    request* r = new request(file);

    //sending request
    r->send();

    std::cout << "HAR: ";
    //if success
    if (r->isOK())
    {
        std::cout << "Success" << std::endl;
        std::cout << "Response Body:" << std::endl;
        //print response
        std::cout << r->getResponse().getBody() << std::endl;
        std::cout << "Response Headers:" << std::endl;
        //print response headers
        std::cout << r->getResponse().getHeaders();
    }
    else
    {
        std::cout << "ERROR: " << r->getError();
    }
    delete r;
    r = nullptr;
    file.close();
  
    curl_global_cleanup();
    return 0;
}
