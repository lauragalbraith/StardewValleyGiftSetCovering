/*
 * Description: program to determine the fewest number of "loved" gifts needed for Stardew Valley villagers
 * Author: Laura Galbraith
*/

#include <iostream> // cout, endl
#include <vector> // vector

#include "valleyfacts.hpp" // Gift, Villager, GiftsByVillager, GiftForVillagers
#include "bucketqueue.hpp" // BucketQueue
// TODO FINALLY ensure no unused imports exist

int main() {
  // Populate gift/villager relationships
  GiftsByVillager gifts_and_villagers = GiftsByVillager(std::vector<Villager>(), std::vector<Gift>());

  // Set up data for set-covering
  std::vector<GiftForVillagers> gifts_for_villagers = gifts_and_villagers.GetGiftSets();

  BucketQueue<GiftForVillagers> bucket_queue = BucketQueue<GiftForVillagers>(gifts_for_villagers);

  // Perform set-covering: https://en.m.wikipedia.org/wiki/Set_cover_problem#Greedy_algorithm
  std::cout << "Gifts needed to give all villagers a Loved gift:" << std::endl;
  unsigned int coverable_villagers = 1;
  do {
    GiftForVillagers next_gift = bucket_queue.GetHighestPrioritySet();
    bucket_queue.DeleteHighestPrioritySet();

    coverable_villagers = next_gift.Size();
    if (coverable_villagers > 0) {
      std::cout << "    " << next_gift << std::endl;
    }
  } while (coverable_villagers > 0);

  // Check that set-covering algorithm did complete given constraints from user input
  const GiftForVillagers covered_villagers = bucket_queue.GetCoveredElements();
  if (covered_villagers.Size() != gifts_and_villagers.GetVillagers().size()) {
    // tell the user that the set-covering algorithm could not complete
    std::cout << std::endl << "Not all villagers can receive a Loved gift with the provided input; these villagers still need a gift: ";

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

  return 0;
}
