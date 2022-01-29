/*
 * Description: implementations to get information on gift/villager relationships
 * Author: Laura Galbraith
*/

#include "valleyfacts.hpp" // self-include header

#include <string> // string
#include <vector> // vector
#include <map> // map
#include <sstream> // stringstream
#include <stdexcept> // runtime_error

#include "curl.hpp" // Curl, CurlResult
#include "xmlparse.hpp" // GetPrecededAndNestedData, XMLParseResult

// Constants
const std::string GiftsByVillager::VILLAGERS_URL = "https://stardewvalleywiki.com/Villagers";
// though there is also a containing div tag, the code cannot currently distinguish between other div tags starting/ending inside of li
const std::vector<std::string> GiftsByVillager::VILLAGERS_CONTAINING_ELEMENTS = {"ul", "li", "p", "a"};

const std::string GiftsByVillager::VILLAGER_URL_PREFIX = "https://stardewvalleywiki.com/";
// this is the format of loved gifts on a Villager's page: <td>Best Gifts: OTHER TEXT</td> ... <td> ... <span>...<a>Spaghetti</a>...</span>...<span>...<a>Peach</a>...</span> ... </td>
const std::vector<std::string> GiftsByVillager::VILLAGER_GIFTS_CONTAINING_ELEMENTS = {"td", "span", "a"};

const std::string GiftsByVillager::FRIENDSHIP_URL = "https://stardewvalleywiki.com/Friendship";

// Populate gift/villager relationships from the Stardew Valley wiki
GiftsByVillager::GiftsByVillager(const std::vector<Villager>& to_skip_villagers, const std::vector<Gift>& to_skip_gifts) {
  // Initiate connection to the SV wiki
  this->curl_interface = new Curl();

  // Initialize internal storage of the parameters
  this->villagers_to_skip.resize(to_skip_villagers.size());
  for (size_t i = 0; i < to_skip_villagers.size(); ++i) {
    this->villagers_to_skip[i] = to_skip_villagers[i];
  }

  this->gifts_to_skip.resize(to_skip_gifts.size());
  for (size_t i = 0; i < to_skip_gifts.size(); ++i) {
    this->gifts_to_skip[i] = to_skip_gifts[i];
  }

  // Combine Villager/Gift data
  const std::vector<Villager> villagers = this->GetVillagers();

  // get gifts that are specifically loved by each villager
  for (auto v:villagers) {
    std::vector<Gift> loved_gifts = this->GetLovedGiftsOfVillager(v);
    for (auto g:loved_gifts) {
      // a mapping of all loved Gifts to the Villagers that love them
      if (this->loved_gifts_of_villagers.find(g) == this->loved_gifts_of_villagers.end()) {
        this->loved_gifts_of_villagers[g] = std::vector<Villager>();
      }

      this->loved_gifts_of_villagers[g].push_back(v);
    }
  }

  // get gifts that are (almost) universally-loved by villagers
  std::map<Gift, std::vector<Villager>> universally_loved_gifts_exceptions = this->GetUniversalLovedGiftExceptions();
  for (auto g_v:universally_loved_gifts_exceptions) {
    if (g_v.second.size() <= 0) {
      this->loved_gifts_of_villagers[g_v.first] = villagers;
    }
    else {
      std::vector<Villager> loving_villagers;

      for (auto v:villagers) {
        bool villager_loves_this = true;
        for (auto excepting_v:g_v.second) {
          if (excepting_v == v) {
            villager_loves_this = false;
            break;
          }
        }

        if (villager_loves_this) {
          loving_villagers.push_back(v);
        }
      }

      this->loved_gifts_of_villagers[g_v.first] = loving_villagers;
    }
  }

  // close down curl memory
  delete this->curl_interface;
  this->curl_interface = NULL;
}

// Get list of villagers from wiki, minus any we want to skip
// TODO incorporate villagers_to_skip once main.cpp takes user input
const std::vector<Villager> GiftsByVillager::GetVillagers() {
  std::vector<Villager> villagers;

  // Make call to wiki
  CurlResult villagers_page = this->curl_interface->CallURL(GiftsByVillager::VILLAGERS_URL.c_str());
  if (villagers_page.error != "" || villagers_page.data == "") {
    throw std::runtime_error("failed to perform URL get of villagers: " + villagers_page.error);
  }

  // parse male marriage candidates
  XMLParseResult bachelor_xml = GetPrecededAndNestedData(villagers_page.data, "h3", "Bachelors", GiftsByVillager::VILLAGERS_CONTAINING_ELEMENTS);
  if (bachelor_xml.error != "" || bachelor_xml.data.size() <= 0) {
    throw std::runtime_error("failed to parse bachelors from villagers page: " + bachelor_xml.error);
  }

  for (auto v:bachelor_xml.data) {
    villagers.push_back(v);
  }

  // parse female marriage candidates
  XMLParseResult bachelorette_xml = GetPrecededAndNestedData(villagers_page.data, "h3", "Bachelorettes", GiftsByVillager::VILLAGERS_CONTAINING_ELEMENTS);
  if (bachelorette_xml.error != "" || bachelorette_xml.data.size() <= 0) {
    throw std::runtime_error("failed to parse bachelorettes from villagers page: " + bachelorette_xml.error);
  }

  for (auto v:bachelorette_xml.data) {
    villagers.push_back(v);
  }

  // parse non-marriage-candidate (but giftable) villagers
  XMLParseResult nonmarriage_xml = GetPrecededAndNestedData(villagers_page.data, "h2", "Non-marriage candidates", GiftsByVillager::VILLAGERS_CONTAINING_ELEMENTS);
  if (nonmarriage_xml.error != "" || nonmarriage_xml.data.size() <= 0) {
    throw std::runtime_error("failed to parse non-marriage candidates from villagers page: " + nonmarriage_xml.error);
  }

  for (auto v:nonmarriage_xml.data) {
    villagers.push_back(v);
  }

  return villagers;
}

// returns a mapping of all loved Gifts of the specified Villager
// TODO incorporate gifts_to_skip once main.cpp takes user input
const std::vector<Gift> GiftsByVillager::GetLovedGiftsOfVillager(const Villager& villager) {
  std::stringstream villager_url;
  villager_url << GiftsByVillager::VILLAGER_URL_PREFIX << villager;

  // Make call to wiki
  CurlResult villager_page = this->curl_interface->CallURL(villager_url.str().c_str());
  if (villager_page.error != "" || villager_page.data == "") {
    throw std::runtime_error("failed to perform URL get of villager " + villager + ": " + villager_page.error);
  }

  XMLParseResult loved_gifts_xml = GetPrecededAndNestedData(villager_page.data, "td", "Best Gifts", GiftsByVillager::VILLAGER_GIFTS_CONTAINING_ELEMENTS);
  if (loved_gifts_xml.error != "" || loved_gifts_xml.data.size() <= 0) {
    throw std::runtime_error("failed to parse gifts from " + villager + "'s page: " + loved_gifts_xml.error);
  }

  return loved_gifts_xml.data;
}

// returns a map of gifts mapped to any villagers that do not love them
// TODO incorporate villagers_to_skip, gifts_to_skip once main.cpp takes user input
const std::map<Gift, std::vector<Villager>> GiftsByVillager::GetUniversalLovedGiftExceptions() {
  // Get page data on (almost) universally-loved gifts
  CurlResult friendship_page = this->curl_interface->CallURL(GiftsByVillager::FRIENDSHIP_URL.c_str());
  if (friendship_page.error != "" || friendship_page.data == "") {
    throw std::runtime_error("failed to perform URL get of friendship page: " + friendship_page.error);
  }

  // parse the list of (almost) universally-loved gifts
  std::vector<std::string> loves_containing_element_names = {"ul", "li", "span", "a"};
  XMLParseResult universal_loves_xml = GetPrecededAndNestedData(friendship_page.data, "h3", "Universal Loves", loves_containing_element_names); // h3 rather than span because there's a tag earlier in the page that also contains it, so it's unique this way
  if (universal_loves_xml.error != "" || universal_loves_xml.data.size() <= 0) {
    throw std::runtime_error("failed to parse universal loves from friendship page: " + universal_loves_xml.error);
  }

  // parse the exceptions to that list; returned in a list form of Villager/Gift/Villager/Gift, with the Villager specified before the Gift they don't love
  std::vector<std::string> exception_containing_element_names = {"ul", "li", "a"};
  XMLParseResult exceptions_xml = GetPrecededAndNestedData(friendship_page.data, "h4", "Universal Loves exceptions", exception_containing_element_names); // h4 rather than span because there's a tag earlier in the page that also contains it, so it's unique this way
  if (exceptions_xml.error != "" || exceptions_xml.data.size() <= 0) {
    throw std::runtime_error("failed to parse exceptions to universal loves from friendship page: " + exceptions_xml.error);
  }

  // combine that data to return
  std::map<Gift, std::vector<Villager>> exceptional_universal_gifts;
  for (auto g:universal_loves_xml.data) {
    std::vector<Villager> unloving_villagers;
    for (size_t exception_i = 1; exception_i < exceptions_xml.data.size(); exception_i += 2) {
      if (exceptions_xml.data[exception_i] == g) {
        unloving_villagers.push_back(exceptions_xml.data[exception_i-1]);
      }
    }

    exceptional_universal_gifts[g] = unloving_villagers;
  }

  return exceptional_universal_gifts;
}
