/*
 * Description: program to determine the fewest number of "loved" gifts needed for Stardew Valley villagers
 * Author: Laura Galbraith
*/

#include <iostream> // cout, endl, ostream
#include <vector> // vector

#include "valleyfacts.hpp" // Villager, Gift, GiftsByVillager
#include "bucketqueue.hpp" // BucketQueue
// TODO FINALLY ensure no unused imports exist

class TestBQType {
  public:
    TestBQType() {}
    TestBQType(const std::string& t): me(t) {}
    unsigned int Size() const { return static_cast<unsigned int>(this->me.size()); }
    bool operator<(const TestBQType& other) const { return this->me < other.me; }
    void AddElements(const TestBQType& other) {
      for (auto c:other.me) {
        if (this->me.find(c) == std::string::npos) {
          this->me.append(1, c);
        }
      }
    }
    size_t RemoveElements(const TestBQType& other) {
      size_t removed = 0;
      for (auto c:other.me) {
        size_t start = this->me.find(c);
        while (start != std::string::npos) {
          this->me.erase(start, 1);
          ++removed;
          start = this->me.find(c);
        }
      }

      return removed;
    }

    std::string GetVal() const { return this->me; }

  private:
    std::string me;
};

// defining the << operator function to ease printing
std::ostream& operator<<(std::ostream& os, const TestBQType& x) {
  os << x.GetVal();
  return os;
}

int main() {
  // Populate gift/villager relationships
  GiftsByVillager gifts_and_villagers = GiftsByVillager(std::vector<Villager>(), std::vector<Gift>());

  // TODO NEXT 51 I'm thinking of instantiating this with a third type, a map<Gift,Villager> so that as I go along it's easy for the bucket class to just do things; or make GiftsByVillager also fit the requirements of BucketQueue<T>
  BucketQueue<TestBQType> str_bucket_queue = BucketQueue<TestBQType>({TestBQType("hap"), TestBQType("hello"), TestBQType("heyo"), TestBQType("sup")});
  std::cout << "MAXIMUM: " << str_bucket_queue.GetHighestPrioritySet() << std::endl;
  str_bucket_queue.DeleteHighestPrioritySet();
  std::cout << "MAXIMUM: " << str_bucket_queue.GetHighestPrioritySet() << std::endl;
  str_bucket_queue.DeleteHighestPrioritySet();
  std::cout << "MAXIMUM: " << str_bucket_queue.GetHighestPrioritySet() << std::endl;
  str_bucket_queue.DeleteHighestPrioritySet();
  std::cout << "MAXIMUM: " << str_bucket_queue.GetHighestPrioritySet() << std::endl;
  str_bucket_queue.DeleteHighestPrioritySet();
  std::cout << "MAXIMUM: should be empty: " << str_bucket_queue.GetHighestPrioritySet() << std::endl;
  str_bucket_queue.DeleteHighestPrioritySet();

  // TODO perform set-covering

  return 0;
}
