/*
 * Description: interfaces to get information on gift/villager relationships
 * Author: Laura Galbraith
*/

#ifndef SVGSC_VALLEY_FACTS_H
#define SVGSC_VALLEY_FACTS_H

#include <string> // string
#include <vector> // vector
#include <map> // map

#include "curl.hpp" // Curl, CurlResult

typedef std::string Villager;
typedef std::string Gift;

class GiftsByVillager {
  public:
    // Constructor
    // will only load giftable villagers not specified in given skip list; likewise with gifts
    GiftsByVillager(const std::vector<Villager>& to_skip_villagers, const std::vector<Gift>& to_skip_gifts);

    // TODO add methods as necessary for interaction with algorithm

  private:
    const std::vector<Villager> GetVillagers();
    const std::vector<Gift> GetLovedGiftsOfVillager(const Villager& villager);
    const std::map<Gift, std::vector<Villager>> GetUniversalLovedGiftExceptions();

    Curl* curl_interface;
    std::vector<Villager> villagers_to_skip;
    std::vector<Gift> gifts_to_skip;
    std::map<Gift, std::vector<Villager>> loved_gifts_of_villagers; // TODO once algorithm is implemented, consider if this is still the most efficient implementation; maybe map of gifts to maps of villagers (to empty structs)?

    static const std::string VILLAGERS_URL;
    static const std::vector<std::string> VILLAGERS_CONTAINING_ELEMENTS;
    static const std::string VILLAGER_URL_PREFIX;
    static const std::vector<std::string> VILLAGER_GIFTS_CONTAINING_ELEMENTS;
    static const std::string FRIENDSHIP_URL;
};

#endif // SVGSC_VALLEY_FACTS_H
