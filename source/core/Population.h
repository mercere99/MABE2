/**
 *  @note This file is part of MABE, https://github.com/mercere/MABE
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

#include "base/vector.h"

#include "Organism.h"

namespace mabe {

  class Population {
  private:
    emp::vector<Organism> orgs;
  public:
    class Position {
      emp::Ptr<Population> pop_ptr;
      size_t pos;
    };
  };

}

#endif
