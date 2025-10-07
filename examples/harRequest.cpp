#include <iostream>
#include <fstream>
#include "request.h"

int main()
{
    //Globally initialize curl
    curl_global_init(CURL_GLOBAL_DEFAULT);


    //Har file example
    std::ifstream reqHr("request.har");

    //File is not open
    if (!reqHr.is_open())
    {
        std::cerr << "Can not open request.har";
        //exit
        return -1;
    }

    //Create a request object with parameters from a file
    request* r = new request(reqHr);

    //send request
    r->send();

    //if success
    if (r->isOK())
    {
        std::cout << "Success!" << std::endl;
        
        //Print response
        std::cout << "Response: " << std::endl;
        std::cout << r->getResponse().getBody() << std::endl;

        //Print response headers
        std::cout << "Response headers: " << std::endl;
        std::cout << r->getResponse().getHeaders() << std::endl;
    }
    else
    {
        std::cerr << "ERROR: " << r->getError();
    }
    
    //Clean up memory
    delete r;

    //Clean up curl globally
    curl_global_cleanup();
    return 0;
}
