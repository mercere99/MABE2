/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Population.h
 *  @brief Container for a group of arbitrary MABE organisms.
 *
 *  Organisms in MABE are stored in Population objects.
 *  A single position in a Population object is described by a Population::Position.
 */

#ifndef MABE_POPULATION_H
#define MABE_POPULATION_H

#include <string>

#include "base/Ptr.h"
#include "base/vector.h"

#include "Organism.h"

namespace mabe {

  class Population {
  private:
    std::string name;                      ///< Unique name for this population.
    int id;                                ///< Position in world of this population.
    emp::vector<emp::Ptr<Organism>> orgs;  ///< Info on all organisms in this population.

  public:
    class Position {
      emp::Ptr<Population> pop_ptr;
      size_t pos;
    };

    Population(const std::string & in_name, int in_id=0) : name(in_name), id(in_id) { }
    Population(const Population & in_pop) : name(in_pop.name + "_copy"), orgs(in_pop.orgs.size()) {
      for (size_t i = 0; i < orgs.size(); i++) {
        orgs[i] = in_pop.orgs[i]->Clone();
      }
    }
    Population(Population &&) = default;

    ~Population() { for (auto x : orgs) x.Delete(); }

    const std::string & GetName() const noexcept { return name; }
    int GetID() const noexcept { return id; }

    void SetID(int in_id) noexcept { id = in_id; }
  };

}

#endif
