/*
 * Description: program to determine the fewest number of "loved" gifts needed for Stardew Valley villagers
 * Author: Laura Galbraith
*/

#include <vector> // vector

#include "valleyfacts.hpp" // Villager, Gift, GiftsByVillager
// TODO FINALLY ensure no unused imports exist

int main() {
  // Populate gift/villager relationships
  GiftsByVillager gifts_and_villagers = GiftsByVillager(std::vector<Villager>(), std::vector<Gift>());

  // TODO NEXT implement templated bucket queue class
  // TODO perform set-covering

  return 0;
}
