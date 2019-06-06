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
    std::string name;
    emp::vector<emp::Ptr<Organism>> orgs;
  public:
    class Position {
      emp::Ptr<Population> pop_ptr;
      size_t pos;
    };

    Population(const std::string & in_name) : name(in_name) { }
    Population(const Population &) = delete;
    Population(Population &&) = default;

    ~Population() { for (auto x : orgs) x.Delete(); }

    const std::string & GetName() const { return name; }
  };

}

#endif
