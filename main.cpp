/*
 * Description: program to determine the fewest number of "loved" gifts needed for Stardew Valley villagers
 * Author: Laura Galbraith
*/

#include <iostream>
#include <string>
#include "curl.hpp" // Curl, CurlResult

// TODO FINALLY ensure no unused imports exist

int main() {
  std::string url = "https://stardewvalleywiki.com/Maru";
  const char* url_ptr = url.c_str();

  Curl* curl_interface = new Curl();
  CurlResult result = curl_interface->CallURL(url_ptr);
  if (result.error != "" || result.data == "") {
    std::cout << "Failed to perform URL get: " << result.error << std::endl;
    return -1;
  }
  delete curl_interface;

  // TODO HERE find out where HTMLparser.h is in apt packages so I can rely on that (note: result.data appears to be padded at start/end with newlines)

  return 0;
}
