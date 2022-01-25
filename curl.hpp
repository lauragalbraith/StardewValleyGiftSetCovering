/*
 * Description: interfaces to make curl calls to URLs
 * Author: Laura Galbraith
*/

#ifndef SVGSC_CURL_H
#define SVGSC_CURL_H

#include <string>
#include <curl/curl.h> // CURL

class CurlResult {
  public:
    // Constructor
    CurlResult(const std::string& result_data, const std::string& result_error);

    // Member variables
    std::string data;
    std::string error; // will be empty if result is successful
};

class Curl {
  public:
    // Default constructor
    Curl();

    // Copy constructor
    Curl(const Curl& other);

    // Assignment operator
    Curl& operator=(const Curl& other);

    // Method to call URL
    CurlResult CallURL(const char* url);

    // Destructor
    ~Curl();

  private:
    void copy(const Curl& other);
    void clear();
    static size_t DataWriter(char* curl_data_ptr, size_t always_one, size_t data_size, std::string* caller_ptr);

    CURL* curl_impl;
};

#endif // SVGSC_CURL_H
