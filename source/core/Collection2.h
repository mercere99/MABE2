/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  Collection.h
 *  @brief A collection of organisms or whole populations; not owner.
 *
 *  While organisms must be managed by Population objects, collections are an easy way
 *  to represent and manipulate groups of organisms (by their position).  Organisms can be
 *  added individully or as whole populations.
 * 
 *  Internally, a Collection is represented by a map with keys of Population pointers and
 *  values of a BitVector indicating the positions in those populations that are included.
 */

#ifndef MABE_COLLECTION_H
#define MABE_COLLECTION_H

#include <set>
#include <string>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/BitVector.h"

#include "Population.h"

namespace mabe {

  class Collection {
  private:
    using pop_ptr_t = emp::Ptr<mabe::Population>;

    /// Information about a single population in this collection.
    struct PopInfo {
      bool full_pop = false;   ///< Should we use the full population?
      emp::BitVector pos_set;  ///< Which positions are we using for this population?

      size_t GetSize(pop_ptr_t pop_ptr) const {
        if (full_pop) return pop_ptr->GetSize();
        return pos_set.CountOnes();
      }
    };

    using map_t = std::map<pop_ptr_t, PopInfo>;
    map_t pos_map;

  public:
    Collection() = default;
    Collection(const Collection &) = default;
    Collection(Collection &&) = default;
    Collection(const Population & pop) { Insert(pop); }
    ~Collection() { }

    Collection & operator=(const Collection &) = default;
    Collection & operator=(Collection &&) = default;

    size_t GetSize() const {
      size_t count = 0;
      for (auto [pop_ptr, pop_info] : pos_map) {
        count += pop_info.GetSize(pop_ptr);
      }
      return count;
    }


    

    /// Add a Population to this collection.
    Collection &  Insert(const Population & pop) {
      for (size_t i = 0; i < pop.GetSize(); i++) {
        insert(OrgPosition(&pop, i));
      }
      return *this;
    }

    /// Add an organism (by position!)
    Collection & Insert(OrgPosition pos) { insert(pos); return *this; }

    /// Add a whole other collection.
    Collection & Insert(const Collection & collection) {
      for (auto pos : collection) insert(pos);
      return *this;
    }

    // @CAO: Add:
    // * Remove()  - works with position or population (or another collection?)
    // * Has() - position
    // * Filter() - takes a function and only keeps positions that return true.
    //   (Filter could even have a three function version that takes a second collection and
    //   indicated what to do with those just in the first collection, just in the second,
    //   or in both.)

    /// Remove all empty positions from this collection.
    Collection & RemoveEmpty() {
      auto it = begin();
      while ( it != end()) {
        if (!it->IsOccupied()) it = erase(it);
        else it++;
      }
      return *this;
    }

    /// Merge this collection with another collection.
    Collection & operator |= (const Collection & collection2) { return Insert(collection2); }

    /// Reduce to the intersection with another colleciton.
    Collection & operator &= (const Collection & collection2) {
      auto it1 = begin();
      auto it2 = collection2.begin();
      while (it1 != end() && it2 != collection2.end()) {
        if (it1 < it2) it1 = erase(it1);  // Not in collection2!
        else if (it2 < it1) it2++;        // Need to catch up iterator for collection2.
        else { it1++; it2++; }            // In both.  Keep and move on.
      }

      return *this;
    }

  };

}

#endif
