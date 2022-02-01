/*
 * Description: interfaces to get information on gift/villager relationships
 * Author: Laura Galbraith
*/

#ifndef SVGSC_VALLEY_FACTS_H
#define SVGSC_VALLEY_FACTS_H

#include <iostream> // ostream
#include <string> // string
#include <vector> // vector
#include <map> // map

#include "curl.hpp" // Curl, CurlResult

typedef std::string Villager;
typedef std::string Gift;

class GiftForVillagers {
  public:
    // Constructors
    GiftForVillagers(); // necessary for GiftForVillagers to be compatible with BucketQueue
    GiftForVillagers(const Gift& g, const std::vector<Villager>& vs);
    GiftForVillagers(const GiftForVillagers& other); // necessary for GiftForVillagers to be compatible with BucketQueue

    // Methods necessary for GiftForVillagers to be compatible with BucketQueue
    GiftForVillagers& operator=(const GiftForVillagers& other);
    bool operator<(const GiftForVillagers& other) const;
    unsigned int Size() const;
    void AddElements(const GiftForVillagers& other);
    size_t RemoveElements(const GiftForVillagers& other);

    // Data-reading methods
    const Gift GetGift() const;
    const std::vector<Villager> GetVillagers() const;

    // Destructor
    ~GiftForVillagers();

  private:
    void copy(const GiftForVillagers& other);
    void clear();

    Gift gift;
    std::map<Villager, bool> villagers;
};

// necessary for GiftForVillagers to be compatible with BucketQueue
std::ostream& operator<<(std::ostream& os, const GiftForVillagers& x);

class GiftsByVillager {
  public:
    // Constructor
    // will only load giftable villagers not specified in given skip list; likewise with gifts
    GiftsByVillager(const std::vector<Villager>& to_skip_villagers, const std::vector<Gift>& to_skip_gifts);

    // return list of all gifts and the villagers associated with them
    std::vector<GiftForVillagers> GetGiftSets() const;

    const std::vector<Villager> GetVillagers();

  private:
    const std::vector<Villager> PopulateVillagersFromWiki();
    const std::vector<Gift> PopulateLovedGiftsOfVillagerFromWiki(const Villager& villager);
    const std::map<Gift, std::vector<Villager>> GetUniversalLovedGiftExceptions();

    Curl* curl_interface;
    std::map<Villager,bool> villagers_to_skip;
    std::map<Gift,bool> gifts_to_skip;
    std::map<Gift, std::vector<Villager>> loved_gifts_of_villagers;

    static const std::string VILLAGERS_URL;
    static const std::vector<std::string> VILLAGERS_CONTAINING_ELEMENTS;
    static const std::string VILLAGER_URL_PREFIX;
    static const std::vector<std::string> VILLAGER_GIFTS_CONTAINING_ELEMENTS;
    static const std::string FRIENDSHIP_URL;
};

#endif // SVGSC_VALLEY_FACTS_H
