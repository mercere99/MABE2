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
 */

#ifndef MABE_COLLECTION_H
#define MABE_COLLECTION_H

#include <set>
#include <string>

#include "base/Ptr.h"
#include "base/vector.h"

#include "Population.h"

namespace mabe {

  class Collection : public std::set<OrgPosition> {
  public:
    Collection() = default;
    Collection(const Collection &) = default;
    Collection(Collection &&) = default;
    Collection(const Population & pop) { Insert(pop); }
    ~Collection() { }

    Collection & operator=(const Collection &) = default;
    Collection & operator=(Collection &&) = default;

    using iterator = std::set<OrgPosition>::iterator;;

    size_t GetSize() const { return size(); }

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
    }

  };

}

#endif
