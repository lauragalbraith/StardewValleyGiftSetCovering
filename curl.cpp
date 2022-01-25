/*
 * Description: implementations to make curl calls to URLs
 * Documentation/references:
 *    - libcurl / curl.h documentation: https://curl.se/libcurl/c/
 *    - libcurl / curl.h example with C++: https://curl.se/libcurl/c/htmltitle.html
 *    - CURLcode errors: https://curl.se/libcurl/c/libcurl-errors.html
 * Author: Laura Galbraith
*/

#include "curl.hpp" // self-include header

#include <string> // string
#include <iostream> // cout, endl
#include <sstream> // stringstream
#include <stdexcept> // runtime_error
#include <stdio.h> // used with CURLOPT_WRITEDATA
#include <curl/curl.h> // CURL, CURLcode, CURL* constants, related methods

CurlResult::CurlResult(const std::string& result_data, const std::string& result_error)
  : data(result_data), error(result_error)
{}

Curl::Curl() {
  this->curl_impl = curl_easy_init();
  if (this->curl_impl == NULL) {
    throw std::runtime_error("curl handle failed to initialize");
  }
}

Curl::Curl(const Curl& other) {
  this->copy(other);
}

Curl& Curl::operator=(const Curl& other) {
  if (&other != this) {
    this->clear();
    this->copy(other);
  }
  return *this;
}

// it's possible to make multiple calls with the same curl object; it keeps the connection open in the meantime
// TODO consider using CURLOPT_ERRORBUFFER if I get errors that I need explained; see https://curl.se/libcurl/c/htmltitle.html
CurlResult Curl::CallURL(const char* url) {
  CURLcode code = curl_easy_setopt(this->curl_impl, CURLOPT_URL, url);
  if (code != CURLE_OK) {
    std::stringstream error_code;
    error_code << "got error code while setting URL (see https://curl.se/libcurl/c/libcurl-errors.html): " << code;
    return CurlResult("", error_code.str());
  }

  code = curl_easy_setopt(this->curl_impl, CURLOPT_WRITEFUNCTION, Curl::DataWriter); // necessary to specify so data goes into the below-specified pointer
  if (code != CURLE_OK) {
    std::stringstream error_code;
    error_code << "got error code while setting data-writing function (see https://curl.se/libcurl/c/libcurl-errors.html): " << code;
    return CurlResult("", error_code.str());
  }

  std::string data;
  code = curl_easy_setopt(this->curl_impl, CURLOPT_WRITEDATA, &data);
  if (code != CURLE_OK) {
    std::stringstream error_code;
    error_code << "got error code while setting where data is written (see https://curl.se/libcurl/c/libcurl-errors.html): " << code;
    return CurlResult("", error_code.str());
  }

  code = curl_easy_perform(this->curl_impl);
  if (code != CURLE_OK) {
    std::stringstream error_code;
    error_code << "got error code while calling URL (see https://curl.se/libcurl/c/libcurl-errors.html): " << code;
    return CurlResult("", error_code.str());
  }

  return CurlResult(data, "");
}

Curl::~Curl() {
  this->clear();
}

void Curl::copy(const Curl& other) {
  std::cout << "DEBUG: in curl copy" << std::endl;
  // calling copy constructor is impossible; since we are not supposed to free the memory associated with it either (https://curl.se/libcurl/c/curl_easy_getinfo.html), we cannot copy internal info...
  // have to copy the same handle
  this->curl_impl = other.curl_impl;
}

void Curl::clear() {
  curl_easy_cleanup(this->curl_impl); // does memory management on CURL type for us
  this->curl_impl = NULL;
}

// based on 'writer' example from https://curl.se/libcurl/c/htmltitle.html
size_t Curl::DataWriter(char* curl_data_ptr, size_t always_one, size_t data_size, std::string* caller_ptr) {
  if (caller_ptr == NULL) {
    return 0;
  }

  size_t size_to_write = data_size * always_one;
  caller_ptr->append(curl_data_ptr, size_to_write);
  return size_to_write;
}
