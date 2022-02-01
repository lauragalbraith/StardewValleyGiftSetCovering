/*
 * Description: program to determine the fewest number of "loved" gifts needed for Stardew Valley villagers
 * Author: Laura Galbraith
*/

#include <iostream> // cout, endl
#include <string> // string
#include <regex> // regex, regex_search, smatch
#include <vector> // vector

#include "valleyfacts.hpp" // Gift, Villager, GiftsByVillager, GiftForVillagers
#include "bucketqueue.hpp" // BucketQueue

// Constants for input format
const std::string SKIP_VILLAGERS_FLAG = "--skip-villagers";
const std::string SKIP_GIFTS_FLAG = "--missing-gifts";
const std::string HELP_FLAG = "--help";
const char INPUT_LIST_SEPARATOR = ',';
const std::regex NAME_RGX("^[a-zA-Zñ ']+$");

void PrintUsage() {
  std::cout << std::endl;
  std::cout << "Usage: <program> ";
  std::cout << "[" << SKIP_VILLAGERS_FLAG << " \"Villager1" << INPUT_LIST_SEPARATOR << "Villager2\"] ";
  std::cout << "[" << SKIP_GIFTS_FLAG << " \"GiftA" << INPUT_LIST_SEPARATOR << "GiftB\"] ";
  std::cout << "[" << HELP_FLAG << "]" << std::endl;
  std::cout << std::endl;
}

// returns true if the given string is a valid villager or gift name
bool ValidName(const std::string& s) {
  std::smatch m;
  std::regex_search(s, m, NAME_RGX);

  if (m.size() < 1) {
    return false;
  }

  return true;
}

// returns empty list if any item in the list is not a valid name
std::vector<std::string> GetSeparatedNameList(const std::string& s) {
  std::vector<std::string> ret;

  size_t start = 0;
  size_t end = 0;
  // search thru string for separator
  while (start < s.size() && end != std::string::npos) {
    // find the end of this element of the separated list
    end = s.find(INPUT_LIST_SEPARATOR, start);

    size_t part_len;
    if (end == std::string::npos) {
      part_len = s.size() - start;
    }
    else {
      part_len = end - start;
    }

    // check the part name is valid
    std::string part = s.substr(start, part_len);
    if (!ValidName(part)) {
      return std::vector<std::string>();
    }

    // add valid part to list
    ret.push_back(part);

    // move to next part of string
    start = end+1;
  }

  return ret;
}

int main(int argc, char *argv[]) {
  // Parse user input
  std::vector<Villager> villagers_to_skip;
  std::vector<Gift> gifts_to_skip;

  int i = 1; // arg 0 is the program name: skip
  while (i < argc) {
    std::string option = std::string(argv[i]);
    if (option == SKIP_VILLAGERS_FLAG) {
      // check there is a following argument
      if (i+1 >= argc) {
        PrintUsage();
        return -1;
      }

      // check the option hasn't been specified already
      if (villagers_to_skip.size() > 0) {
        PrintUsage();
        return -1;
      }

      // parse list
      villagers_to_skip = GetSeparatedNameList(std::string(argv[i+1]));
      if (villagers_to_skip.size() <= 0) {
        PrintUsage();
        return -1;
      }

      // move past 2-part arg
      i += 2;
    }
    else if (option == SKIP_GIFTS_FLAG) {
      // check there is a following argument
      if (i+1 >= argc) {
        PrintUsage();
        return -1;
      }

      // check the option hasn't been specified already
      if (gifts_to_skip.size() > 0) {
        PrintUsage();
        return -1;
      }

      // parse list
      gifts_to_skip = GetSeparatedNameList(std::string(argv[i+1]));
      if (gifts_to_skip.size() <= 0) {
        PrintUsage();
        return -1;
      }

      // move past 2-part arg
      i += 2;
    }
    else {
      PrintUsage();
      return option == HELP_FLAG ? 0 : -1;
    }
  }

  // Populate gift/villager relationships
  GiftsByVillager gifts_and_villagers = GiftsByVillager(villagers_to_skip, gifts_to_skip);

  // Set up data for set-covering
  std::vector<GiftForVillagers> gifts_for_villagers = gifts_and_villagers.GetGiftSets();

  BucketQueue<GiftForVillagers> bucket_queue = BucketQueue<GiftForVillagers>(gifts_for_villagers);

  // Perform set-covering: https://en.m.wikipedia.org/wiki/Set_cover_problem#Greedy_algorithm
  std::vector<GiftForVillagers> loved_gifts;
  unsigned int coverable_villagers = 1;
  do {
    GiftForVillagers next_gift = bucket_queue.GetHighestPrioritySet();
    bucket_queue.DeleteHighestPrioritySet();

    coverable_villagers = next_gift.Size();
    if (coverable_villagers > 0) {
      loved_gifts.push_back(next_gift);
    }
  } while (coverable_villagers > 0);

  // Check that set-covering algorithm did complete given constraints from user input
  const GiftForVillagers covered_villagers = bucket_queue.GetCoveredElements();
  if (covered_villagers.Size() != gifts_and_villagers.GetVillagers().size()) {
    // tell the user that the set-covering algorithm could not complete
    std::cout << "Not all villagers can receive a 'loved' gift with the provided input; these villagers could receive a 'liked' gift instead: ";

    // print what villagers remain uncovered
    GiftForVillagers all_villagers = GiftForVillagers("", gifts_and_villagers.GetVillagers());
    all_villagers.RemoveElements(covered_villagers);
    std::vector<Villager> remaining_villagers = all_villagers.GetVillagers();
    for (std::vector<Villager>::size_type villager_i = 0; villager_i < remaining_villagers.size(); ++villager_i) {
      std::cout << remaining_villagers[villager_i];
      if (villager_i != remaining_villagers.size()-1) {
        std::cout << ", ";
      }
    }
    std::cout << std::endl;
  }

  // Display the optimal total of gifts
  std::cout << std::endl << "Gifts needed to give ";
  std::cout << (covered_villagers.Size() != gifts_and_villagers.GetVillagers().size() ? "all possible" : "all");
  std::cout << " villagers a 'loved' gift:" << std::endl;

  for (auto g:loved_gifts) {
    std::cout << "    " << g << std::endl;
  }
  std::cout << std::endl;

  return 0;
}
