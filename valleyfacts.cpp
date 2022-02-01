/*
 * Description: implementations to get information on gift/villager relationships
 * Author: Laura Galbraith
*/

#include "valleyfacts.hpp" // self-include header

#include <iostream> // ostream
#include <string> // string
#include <vector> // vector
#include <map> // map
#include <sstream> // stringstream
#include <stdexcept> // runtime_error

#include "curl.hpp" // Curl, CurlResult
#include "xmlparse.hpp" // GetPrecededAndNestedData, XMLParseResult

GiftForVillagers::GiftForVillagers() {
  this->gift = "";
}

GiftForVillagers::GiftForVillagers(const Gift& g, const std::vector<Villager>& vs)
  : gift(g)
{
  for (auto v:vs) {
    this->villagers[v] = true; // the 'true' value does not affect the algorithm
  }
}

GiftForVillagers::GiftForVillagers(const GiftForVillagers& other) {
  this->copy(other);
}

GiftForVillagers& GiftForVillagers::operator=(const GiftForVillagers& other) {
  if (&other != this) {
    this->clear();
    this->copy(other);
  }
  return *this;
}

bool GiftForVillagers::operator<(const GiftForVillagers& other) const {
  if (this->gift < other.gift) {
    return true;
  }
  else if (this->gift == other.gift) {
    if (this->villagers.size() < other.villagers.size()) {
      return true;
    }
    else if (this->villagers.size() == other.villagers.size()) {
      auto this_it = this->villagers.cbegin();
      auto other_it = other.villagers.cbegin();
      while (this_it != this->villagers.end() && other_it != other.villagers.cend()) {
        if (this_it->first == other_it->first) {
          ++this_it;
          ++other_it;
          continue; // see if there's a significant difference in the next element
        }

        // otherwise there is a significant difference between the elements, which determines the ordering
        return this_it->first < other_it->first;
      }
    }
  }

  return false;
}

// O(1)
unsigned int GiftForVillagers::Size() const {
  return static_cast<unsigned int>(this->villagers.size());
}

// "elements" for this class are villagers
// O(number of elements of 'other')
void GiftForVillagers::AddElements(const GiftForVillagers& other) {
  for (auto v:other.villagers) {
    this->villagers[v.first] = true; // the 'true' value does not affect the algorithm
  }
}

// "elements" for this class are villagers
// O(number of elements of 'other')
size_t GiftForVillagers::RemoveElements(const GiftForVillagers& other) {
  size_t villagers_removed = 0;
  for (auto v:other.villagers) {
    if (this->villagers.find(v.first) != this->villagers.end()) {
      this->villagers.erase(v.first);
      ++villagers_removed;
    }
  }

  return villagers_removed;
}

const Gift GiftForVillagers::GetGift() const {
  return this->gift;
}

const std::vector<Villager> GiftForVillagers::GetVillagers() const {
  std::vector<Villager> ret;
  ret.resize(this->villagers.size());

  int i = 0;
  for (auto v:this->villagers) {
    ret[i] = v.first;
    ++i;
  }

  return ret;
}

GiftForVillagers::~GiftForVillagers() {
  this->clear();
}

void GiftForVillagers::copy(const GiftForVillagers& other) {
  // copy gift
  this->gift = other.gift;

  // copy villagers
  for (auto v:other.villagers) {
    this->villagers[v.first] = v.second;
  }
}

void GiftForVillagers::clear() {
  this->gift = "";
  this->villagers.clear();
}

// ex. "4 Prismatic Shard items for Pierre, Lewis, Penny, Marnie"
std::ostream& operator<<(std::ostream& os, const GiftForVillagers& x) {
  // print gift
  os << x.Size() << " " << x.GetGift() << " item";
  if (x.Size() > 1) {
    os << "s";
  }

  os << " for ";

  // print villagers
  const std::vector<Villager> vs = x.GetVillagers();
  for (std::vector<Villager>::size_type villager_i = 0; villager_i < vs.size(); ++villager_i) {
    os << vs[villager_i];
    if (villager_i != vs.size()-1) {
      os << ", ";
    }
  }

  if (vs.size() == 0) {
    os << "no one";
  }

  return os;
}

// Constants for wiki usage
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
  // (the bool value in the maps will represent whether they have been seen from the wiki or not)
  for (auto v:to_skip_villagers) {
    this->villagers_to_skip[v] = false;
  }

  for (auto g:to_skip_gifts) {
    this->gifts_to_skip[g] = false;
  }

  // Combine Villager/Gift data
  const std::vector<Villager> villagers = this->PopulateVillagersFromWiki();

  // get gifts that are specifically loved by each villager
  for (auto v:villagers) {
    std::vector<Gift> loved_gifts = this->PopulateLovedGiftsOfVillagerFromWiki(v);
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

std::vector<GiftForVillagers> GiftsByVillager::GetGiftSets() const {
  std::vector<GiftForVillagers> ret;
  ret.resize(this->loved_gifts_of_villagers.size());

  int i = 0;
  for (auto g_v:this->loved_gifts_of_villagers) {
    ret[i] = GiftForVillagers(g_v.first, g_v.second);
    ++i;
  }

  return ret;
}

// Get list of villagers stored
const std::vector<Villager> GiftsByVillager::GetVillagers() {
  // de-duplicate villagers
  std::map<Villager, bool> v_map;
  for (auto g_v:this->loved_gifts_of_villagers) {
    for (auto v:g_v.second) {
      v_map[v] = true;
    }
  }

  // turn into list
  std::vector<Villager> ret;
  ret.resize(v_map.size());
  int i = 0;
  for (auto v:v_map) {
    ret[i] = v.first;
    ++i;
  }

  return ret;
}

// Get list of villagers from wiki, minus any we want to skip
const std::vector<Villager> GiftsByVillager::PopulateVillagersFromWiki() {
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
    // add the villager to the total list if they aren't being skipped
    if (this->villagers_to_skip.find(v) != this->villagers_to_skip.end()) {
      this->villagers_to_skip[v] = true; // mark seen while parsing wiki
      continue;
    }

    villagers.push_back(v);
  }

  // parse female marriage candidates
  XMLParseResult bachelorette_xml = GetPrecededAndNestedData(villagers_page.data, "h3", "Bachelorettes", GiftsByVillager::VILLAGERS_CONTAINING_ELEMENTS);
  if (bachelorette_xml.error != "" || bachelorette_xml.data.size() <= 0) {
    throw std::runtime_error("failed to parse bachelorettes from villagers page: " + bachelorette_xml.error);
  }

  for (auto v:bachelorette_xml.data) {
    // add the villager to the total list if they aren't being skipped
    if (this->villagers_to_skip.find(v) != this->villagers_to_skip.end()) {
      this->villagers_to_skip[v] = true; // mark seen while parsing wiki
      continue;
    }

    villagers.push_back(v);
  }

  // parse non-marriage-candidate (but giftable) villagers
  XMLParseResult nonmarriage_xml = GetPrecededAndNestedData(villagers_page.data, "h2", "Non-marriage candidates", GiftsByVillager::VILLAGERS_CONTAINING_ELEMENTS);
  if (nonmarriage_xml.error != "" || nonmarriage_xml.data.size() <= 0) {
    throw std::runtime_error("failed to parse non-marriage candidates from villagers page: " + nonmarriage_xml.error);
  }

  for (auto v:nonmarriage_xml.data) {
    // add the villager to the total list if they aren't being skipped
    if (this->villagers_to_skip.find(v) != this->villagers_to_skip.end()) {
      this->villagers_to_skip[v] = true; // mark seen while parsing wiki
      continue;
    }

    villagers.push_back(v);
  }

  return villagers;
}

// returns a mapping of all loved Gifts of the specified Villager
const std::vector<Gift> GiftsByVillager::PopulateLovedGiftsOfVillagerFromWiki(const Villager& villager) {
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

  std::vector<Gift> acceptable_gifts;
  for (auto g:loved_gifts_xml.data) {
    // add the gift to the total list if it isn't being skipped
    if (this->gifts_to_skip.find(g) != this->gifts_to_skip.end()) {
      this->gifts_to_skip[g] = true; // mark seen while parsing wiki
      continue;
    }

    acceptable_gifts.push_back(g);
  }

  return acceptable_gifts;
}

// returns a map of gifts mapped to any villagers that do not love them
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
    // process the gift if it isn't being skipped
    if (this->gifts_to_skip.find(g) != this->gifts_to_skip.end()) {
      this->gifts_to_skip[g] = true; // mark seen while parsing wiki
      continue;
    }

    std::vector<Villager> unloving_villagers;
    for (size_t exception_i = 1; exception_i < exceptions_xml.data.size(); exception_i += 2) {
      // process the exceptional villager if they aren't being skipped
      if (this->villagers_to_skip.find(exceptions_xml.data[exception_i-1]) != this->villagers_to_skip.end()) {
        this->villagers_to_skip[exceptions_xml.data[exception_i-1]] = true; // make seen while parsing wiki
        continue;
      }

      if (exceptions_xml.data[exception_i] == g) {
        unloving_villagers.push_back(exceptions_xml.data[exception_i-1]);
      }
    }

    exceptional_universal_gifts[g] = unloving_villagers;
  }

  return exceptional_universal_gifts;
}
