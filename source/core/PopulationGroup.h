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

  class PopulationGroup;

  class OrgPosition_Group : public OrgPosition {
  private:
    emp::Ptr<PopulationGroup> group_ptr;
    uint16_t pop_id;

  public:
    OrgPosition_Group(PopulationGroup & _group,
                      size_t _pop_id=0,
                      size_t _pos=0,
                      bool _skip=false);
    OrgPosition_Group(const OrgPosition_Group &) = default;
    OrgPosition_Group(OrgPosition_Group &&) = default;
    OrgPosition_Group & operator=(const OrgPosition_Group &) = default;
    OrgPosition_Group & operator=(OrgPosition_Group &&) = default;
  };

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

    /// Return how many populations are in this group.
    size_t GetNumPops() const { return pops.size(); }

    /// Return a specific population.
    Population & GetPopulation(size_t pop_id) { return pops[pop_id]; }

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


  OrgPosition_Group::OrgPosition_Group(PopulationGroup & _group, size_t _pop_id,
                                       size_t _pos, bool _skip)
    : OrgPosition(nullptr, 0, _skip)
    , group_ptr(&_group), pop_id(_pop_id)
  {
    if (group_ptr->GetNumPops() > pop_id) {
      pop_ptr = &_group.GetPopulation(pop_id);
      pos = _pos;
    }
  }

}

#endif
