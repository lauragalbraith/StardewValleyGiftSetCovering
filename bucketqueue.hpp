/*
 * Description: interface and implementation to a templated bucket queue
 * Documentation: of abstract type: https://en.m.wikipedia.org/wiki/Bucket_queue
 *                of use with set-cover: https://en.m.wikipedia.org/wiki/Bucket_queue#Greedy_set_cover
 * Author: Laura Galbraith
*/

#ifndef SVGSC_BUCKET_QUEUE_H
#define SVGSC_BUCKET_QUEUE_H

#include <vector> // vector
#include <map> // map
#include <tuple> // pair
#include <stdexcept> // invalid_argument, range_error, logic_error

template <class T> // class T requirements: default constructor, copy constructor, operator=, operator<, std::ostream& operator<<(std::ostream& os, const T& t), unsigned int Size() const, void AddElements(const T& other), size_t RemoveElements(const T& other)
class BucketQueue {
  public:
    // Constructors
    BucketQueue();
    BucketQueue(const std::vector<T>& initial_sets);

    // Abstract type methods
    void InsertSet(T set);
    T GetHighestPrioritySet(); // if there is no highest-priority (no buckets left), the default value of T (T()) is returned
    void DeleteHighestPrioritySet();
    void DecreasePriorityOfSet(T set, unsigned int old_priority, unsigned int new_priority);

  private:
    std::pair<int, T> GetHighestPriorityPosition() const;
    void DeleteSet(T set, unsigned int priority);
    void ResizeBuckets();

    // "indexed by priorities, whose cells contain collections of items with the same priority as each other" (wikipedia)
    std::vector<std::map<T, bool>> buckets;
    T covered_set;
};

// default constructor takes in no elements
template <class T>
BucketQueue<T>::BucketQueue() {
  this->buckets.resize(0);
  this->covered_set = T();
}

// this constructor initializes the set by inserting elements for the user
// O(number of initial sets), as long as T Size() method is O(1)
template <class T>
BucketQueue<T>::BucketQueue(const std::vector<T>& initial_sets) {
  // fill in buckets with initial_sets
  for (auto set:initial_sets) {
    this->InsertSet(set);
  }

  this->covered_set = T();
}

// O(1), as long as T Size() method is O(1)
template <class T>
void BucketQueue<T>::InsertSet(T set) {
  const unsigned int set_priority = set.Size();
  // resize buckets as needed
  if (set_priority >= this->buckets.size()) {
    this->buckets.resize(set_priority+1);
  }

  // store set in bucket
  // the 'true' value does not affect the algorithm
  this->buckets[set_priority][set] = true;
}

// O(1)
template <class T>
T BucketQueue<T>::GetHighestPrioritySet() {
  std::pair<int, T> max_pos = this->GetHighestPriorityPosition();
  return max_pos.second;
}

// amortized O(number of elements) - https://en.m.wikipedia.org/wiki/Bucket_queue#Greedy_set_cover
template <class T>
void BucketQueue<T>::DeleteHighestPrioritySet() {
  std::pair<int, T> max_pos = this->GetHighestPriorityPosition();
  // check for invalid position, to see if there even is a maximum
  if (max_pos.first < 0) {
    return;
  }

  // delete the highest-priority set
  this->DeleteSet(max_pos.second, max_pos.first);

  // add the deleted set's elements to the so-far-covered-set
  this->covered_set.AddElements(max_pos.second);

  // adjust priorities of remaining sets, based on what the now-deleted set covered
  for (typename std::vector<std::map<T, bool>>::size_type bucket_i = 0; bucket_i < this->buckets.size(); ++bucket_i) {
    typename std::map<T, bool>::iterator set_it = this->buckets[bucket_i].begin();
    while (set_it != this->buckets[bucket_i].end()) {
      T set_in_map = set_it->first;
      T updated_set = set_in_map;

      // update the set to not include any elements already covered
      size_t elems_removed = updated_set.RemoveElements(this->covered_set);

      // check how to handle bucket iteration/the set
      if (elems_removed > 0) {
        // update the key stored in the map, so that DecreasePriorityOfSet will find the updated element
        typename std::map<T, bool>::size_type elems_deleted = this->buckets[bucket_i].erase(set_in_map);
        if (elems_deleted != 1) {
          throw std::logic_error("original set with covered elements not found in bucket");
        }

        this->buckets[bucket_i][updated_set] = true;

        // follow usual flow of changing set priority
        this->DecreasePriorityOfSet(updated_set, static_cast<unsigned int>(bucket_i), updated_set.Size());

        // restart the set iteration now that the iterator is invalid
        set_it = this->buckets[bucket_i].begin();
      }
      else {
        // no changes to the set; proceed to next set
        ++set_it;
      }
    }
  }
}

// O(1)
template <class T>
void BucketQueue<T>::DecreasePriorityOfSet(T set, unsigned int old_priority, unsigned int new_priority) {
  if (new_priority == old_priority) {
    return;
  }
  else if (new_priority > old_priority) {
    throw std::invalid_argument("bucket queue priority can only be decreased");
  }

  // remove from the buckets at its old priority (and resize buckets as necessary)
  this->DeleteSet(set, old_priority);

  // re-add it to buckets at its new priority
  this->InsertSet(set);
}

// Return the vector position inside buckets where highest priority can be found, and the chosen element therein
// O(1)
template <class T>
std::pair<int, T> BucketQueue<T>::GetHighestPriorityPosition() const {
  int max_i = static_cast<int>(this->buckets.size())-1;
  T set = T();

  if (max_i >= 0) {
    set = this->buckets[max_i].begin()->first;
  }

  return std::pair<int, T>(max_i, set);
}

// O(number of priorities)
template <class T>
void BucketQueue<T>::DeleteSet(T set, unsigned int priority) {
  if (priority >= this->buckets.size()) {
    throw std::range_error("priority set-to-delete is too large");
  }

  // delete set
  typename std::map<T, bool>::size_type elems_deleted = this->buckets[priority].erase(set);
  if (elems_deleted != 1) {
    throw std::invalid_argument("set-to-delete not found at priority");
  }

  if (priority == this->buckets.size()-1) {
    this->ResizeBuckets();
  }
}

// O(number of priorities)
template <class T>
void BucketQueue<T>::ResizeBuckets() {
  // resize buckets vector to exactly fit remaining highest priority
  for (typename std::vector<std::map<T, bool>>::size_type bucket_i = this->buckets.size()-1; bucket_i >= 0; --bucket_i) {
    if (this->buckets[bucket_i].size() > 0) {
      this->buckets.resize(bucket_i+1);
      return;
    }
  }
}

#endif // SVGSC_BUCKET_QUEUE_H
