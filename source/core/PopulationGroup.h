/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  PopulationGroup.h
 *  @brief A collection of populations for easy iteration.
 *
 *  In many cases we will want multiple populations treated like a single population.  This
 *  class simplifies handling groups of populations.
 */

#ifndef MABE_POPULATION_GROUP_H
#define MABE_POPULATION_GROUP_H

#include <string>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/vector_utils.h"

#include "Population.h"

namespace mabe {

  class PopulationGroup {
  private:
    emp::vector< emp::Ptr<Population> > pops;

  public:
    PopulationGroup() = default;
    PopulationGroup(const PopulationGroup &) = default;
    PopulationGroup(PopulationGroup &&) = default;
    PopulationGroup(Population & in_pop) { pops.push_back(&in_pop); }
    ~PopulationGroup() { }

    PopulationGroup & operator=(const PopulationGroup &) = default;
    PopulationGroup & operator=(PopulationGroup &&) = default;

    /// Return the total size of all of the populations combined.
    size_t GetSize() const {
      size_t count = 0;
      for (auto pop_ptr : pops) {
        count += pop_ptr->GetSize();
      }
      return count;
    }

    /// Add a Population to this group.
    PopulationGroup &  Insert(Population & pop) {
      emp_assert( !emp::Has(pops, &pop) );
      pops.push_back(&pop);
      return *this;
    }

    /// Add a whole other group.
    PopulationGroup & Merge(const PopulationGroup & group) {
      for (auto in_pop : group.pops) {
        if (!emp::Has(pops, in_pop)) pops.push_back(in_pop);
      }
      return *this;
    }

    // @CAO: Add:
    // * Remove() - population
    // * Has() - population
    // * operator()
    // * iterators & associated functions


  };

}

#endif
