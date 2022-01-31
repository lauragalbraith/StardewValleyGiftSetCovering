/*
 * Description: program to determine the fewest number of "loved" gifts needed for Stardew Valley villagers
 * Author: Laura Galbraith
*/

#include <iostream> // cout, endl
#include <vector> // vector

#include "valleyfacts.hpp" // GiftsByVillager, GiftForVillagers
#include "bucketqueue.hpp" // BucketQueue
// TODO FINALLY ensure no unused imports exist

int main() {
  // Populate gift/villager relationships
  GiftsByVillager gifts_and_villagers = GiftsByVillager(std::vector<Villager>(), std::vector<Gift>());

  // Set up data for set-covering
  std::vector<GiftForVillagers> gifts_for_villagers = gifts_and_villagers.GetGiftSets();

  BucketQueue<GiftForVillagers> bucket_queue = BucketQueue<GiftForVillagers>(gifts_for_villagers);

  // TODO Perform set-covering
  std::cout << bucket_queue.GetHighestPrioritySet() << std::endl;

  return 0;
}
