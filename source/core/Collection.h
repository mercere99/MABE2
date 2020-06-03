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

  class Collection {
  private:
    std::set<OrgPosition> orgs;

  public:
    Collection() = default;
    Collection(const Collection &) = default;
    Collection(Collection &&) = default;
    ~Collection() { }

    Collection & operator=(const Collection &) = default;
    Collection & operator=(Collection &&) = default;

    size_t GetSize() const { return orgs.size(); }

    /// Add a Population to this collection.
    void AddPop(Population & pop) {
      for (size_t i = 0; i < pop.GetSize(); i++) {
        orgs.insert(OrgPosition(&pop, i));
      }
    }

    /// Add an organism (by position!)
    void AddOrg(OrgPosition pos) { orgs.insert(pos); }
  };

}

#endif
