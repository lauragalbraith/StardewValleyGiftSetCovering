/*
 * Description: program to determine the fewest number of "loved" gifts needed for Stardew Valley villagers
 * Author: Laura Galbraith
*/

#include <iostream> // cout, endl
#include <string> // string
#include <vector> // vector
#include "curl.hpp" // Curl, CurlResult
#include "xmlparse.hpp" // GetPrecededAndNestedData, XMLParseResult

// TODO FINALLY ensure no unused imports exist

int main() {
  // Testing for now

  // Use curl to get page data
  std::string url = "https://stardewvalleywiki.com/Maru";
  const char* url_ptr = url.c_str();

  Curl* curl_interface = new Curl();
  CurlResult curl_result = curl_interface->CallURL(url_ptr);
  if (curl_result.error != "" || curl_result.data == "") {
    std::cout << "Failed to perform URL get: " << curl_result.error << std::endl;
    return -1;
  }

  // Use libxml2 to parse page data
  // this is the format of loved gifts on a Villager's page: <td>Best Gifts: OTHER TEXT</td> ... <td> ... <span>...<a>Spaghetti</a>...</span>...<span>...<a>Peach</a>...</span> ... </td>
  std::vector<std::string> containing_element_names;
  containing_element_names.push_back("td");
  containing_element_names.push_back("span");
  containing_element_names.push_back("a");

  XMLParseResult page_result = GetPrecededAndNestedData(curl_result.data, "td", "Best Gifts", containing_element_names);
  if (page_result.error != "" || page_result.data.size() <= 0) {
    std::cout << "Failed to parse page data: " << page_result.error << std::endl;
    return -1;
  }

  std::cout << "Desired data from " << url << ":" << std::endl;
  for (auto d:page_result.data) {
    std::cout << d << std::endl;
  }

  std::string universal_url = "https://stardewvalleywiki.com/Friendship";
  CurlResult curl_result2 = curl_interface->CallURL(universal_url.c_str());
  if (curl_result2.error != "" || curl_result2.data == "") {
    std::cout << "Failed to perform URL get: " << curl_result2.error << std::endl;
    return -1;
  }

  std::vector<std::string> containing_element_names2;
  containing_element_names2.push_back("ul");
  containing_element_names2.push_back("li");
  containing_element_names2.push_back("span");
  containing_element_names2.push_back("a");

  XMLParseResult page_result2 = GetPrecededAndNestedData(curl_result2.data, "h3", "Universal Loves", containing_element_names2); // h3 rather than span because there's a tag earlier in the page that also contains it, so it's unique this way
  if (page_result2.error != "" || page_result2.data.size() <= 0) {
    std::cout << "Failed to parse page data: " << page_result2.error << std::endl;
    return -1;
  }

  std::cout << "Desired 'universal loves' data from " << universal_url << ":" << std::endl;
  for (auto d:page_result2.data) {
    std::cout << d << std::endl;
  }

  std::vector<std::string> containing_element_names3;
  containing_element_names3.push_back("ul");
  containing_element_names3.push_back("li");
  containing_element_names3.push_back("a");

  XMLParseResult page_result3 = GetPrecededAndNestedData(curl_result2.data, "h4", "Universal Loves exceptions", containing_element_names3); // h4 rather than span because there's a tag earlier in the page that also contains it, so it's unique this way
  if (page_result3.error != "" || page_result3.data.size() <= 0) {
    std::cout << "Failed to parse page data: " << page_result3.error << std::endl;
    return -1;
  }

  std::cout << "Desired exception data from " << universal_url << ":" << std::endl;
  for (auto d:page_result3.data) {
    std::cout << d << std::endl;
  }

  std::string villagers_url = "https://stardewvalleywiki.com/Villagers";
  CurlResult curl_result3 = curl_interface->CallURL(villagers_url.c_str());
  if (curl_result3.error != "" || curl_result3.data == "") {
    std::cout << "Failed to perform URL get: " << curl_result3.error << std::endl;
    return -1;
  }

  std::cout << "Full " << villagers_url << " HTML page:" << std::endl;
  std::cout << curl_result3.data << std::endl;

  std::vector<std::string> villager_containing_names;
  villager_containing_names.push_back("ul");
  villager_containing_names.push_back("li"); // though there is also a containing div tag, the code cannot currently distinguish between other div tags starting/ending inside of li
  villager_containing_names.push_back("p");
  villager_containing_names.push_back("a");
  XMLParseResult bachelor_xml = GetPrecededAndNestedData(curl_result3.data, "h3", "Bachelors", villager_containing_names);
  if (bachelor_xml.error != "" || bachelor_xml.data.size() <= 0) {
    std::cout << "Failed to parse page data: " << bachelor_xml.error << std::endl;
    return -1;
  }

  std::cout << "Desired bachelor data from " << universal_url << ":" << std::endl;
  for (auto d:bachelor_xml.data) {
    std::cout << d << std::endl;
  }

  XMLParseResult bachelorette_xml = GetPrecededAndNestedData(curl_result3.data, "h3", "Bachelorettes", villager_containing_names);
  if (bachelorette_xml.error != "" || bachelorette_xml.data.size() <= 0) {
    std::cout << "Failed to parse page data: " << bachelorette_xml.error << std::endl;
    return -1;
  }

  std::cout << "Desired bachelorette data from " << universal_url << ":" << std::endl;
  for (auto d:bachelorette_xml.data) {
    std::cout << d << std::endl;
  }

  XMLParseResult nonmarriage_xml = GetPrecededAndNestedData(curl_result3.data, "h2", "Non-marriage candidates", villager_containing_names);
  if (nonmarriage_xml.error != "" || nonmarriage_xml.data.size() <= 0) {
    std::cout << "Failed to parse page data: " << nonmarriage_xml.error << std::endl;
    return -1;
  }

  std::cout << "Desired non-marriage villager data from " << universal_url << ":" << std::endl;
  for (auto d:nonmarriage_xml.data) {
    std::cout << d << std::endl;
  }

  // Clean up memory
  delete curl_interface;

  return 0;
}
