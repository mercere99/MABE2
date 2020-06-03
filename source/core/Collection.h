/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  Collection.h
 *  @brief A collection of organisms or whole populations; not owner.
 *
 *  While organisms must be managed by Population objects, collections are an easy way
 *  to represent and manipulate groups of organisms, either as whole populations or as
 *  sets of individuals.
 */

#ifndef MABE_COLLECTION_H
#define MABE_COLLECTION_H

#include <string>

#include "base/Ptr.h"
#include "base/vector.h"

#include "Population.h"

namespace mabe {

  class Collection {
  private:
    using pop_ptr_t = emp::Ptr<mabe::Population>;
    emp::vector<pop_ptr_t> pops;
    emp::vector<OrgPosition> orgs;

  public:
    Collection() = default;
    Collection(const Collection &) = default;
    Collection(Collection &&) = default;
    ~Collection() { }

    Collection & operator=(const Collection &) = default;
    Collection & operator=(Collection &&) = default;

    size_t GetSize() const {
      size_t count = orgs.size();
      for (auto pop_ptr : pops) count += pop_ptr->GetSize();
      return count;
    }

    /// Add a Population to this collection.
    void AddPop(Population & in_pop) { pops.push_back(&in_pop); }

    /// Add an organism (by position!)
    void AddOrg(OrgPosition pos) { orgs.push_back(pos); }
  };

}

#endif
