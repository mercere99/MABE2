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

      void InsertPos(size_t pos) {
        // Make sure we have room for this position and then set it.
        if (pos_set.GetSize() <= pos) pos_set.Resize(pos+1);
        pos_set.Set(pos);
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

    /// Calculation the total number of positions represented in this collection.
    size_t GetSize() const {
      size_t count = 0;
      for (auto [pop_ptr, pop_info] : pos_map) {
        count += pop_info.GetSize(pop_ptr);
      }
      return count;
    }

    /// Add a Population to this collection.
    Collection &  Insert(const Population & pop) {
      pos_map[&pop].full_pop = true;
      return *this;
    }

    /// Add an organism (by position!)
    Collection & Insert(OrgPosition pos) {
      pos_map[pos.PopPtr()].InsertPos(pos.Pos());
      return *this;
    }

    /// Add a whole other collection.
    Collection & Insert(const Collection & in_collection) {
      for (auto & [pop_ptr, in_pop_info] : in_collection.pos_map) {
        PopInfo & pop_info = pos_map[pop_ptr];

        if (pop_info.full_pop) continue;  // This population is already full.

        // If we're adding a full population, do so.
        if (in_pop_info.full_pop) { pop_info.full_pop = true; continue; }

        // Otherwise add just the entries we need to.

        emp::BitVector & pos_set = pop_info.pos_set;
        emp::BitVector in_pos_set = in_pop_info.pos_set;

        // First, make sure both position sets are the same size.
        if (in_pos_set.GetSize() < pos_set.GetSize()) {
          in_pos_set.Resize(pos_set.GetSize());
        }
        else if (pos_set.GetSize() < in_pos_set.GetSize()) {
          pos_set.Resize(in_pos_set.GetSize());
        }

        // Use 'OR' to join the sets.
        pos_set |= in_pos_set;
      }

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
      for (auto & [pop_ptr, pop_info] : pos_map) {
        emp::BitVector & pos_set = pop_info.pos_set;

        // If this population is full, switch it over to use the bitmap.
        if (pop_info.full_pop) {
          pos_set.Resize(pop_ptr->GetSize());
          pos_set.SetAll();
          pop_info.full_pop = false;
        }

        // Scan through organisms, removing inclusion of those that are empty.
        for (int pos = pos_set.FindBit(); pos != -1; pos = pos_set.FindBit(pos)) {
          if (!pop_ptr->IsOccupied((size_t) pos)) pos_set.Set(pos,false);
        }
      }
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
