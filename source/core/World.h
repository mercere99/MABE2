/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  World.h
 *  @brief Controller for an individual World in MABE.
 * 
 * A WORLD maintains one or more populations or organisms, "evaluate" modules
 * to measure phenotypic traits, and a "select" modules to use those traits to
 * affect/determine the organism's reproductive success.
 */

#ifndef MABE_WORLD_H
#define MABE_WORLD_H

#include "base/Ptr.h"
#include "base/vector.h"

#include "EvaluateModule.h"
#include "Population.h"
#include "SelectModule.h"

namesapce mabe {

  class World {
    emp::vector<mabe::Population> pops;
    emp::vector<emp::Ptr<EvaluateModule>> evals;
    emp::vector<emp::Ptr<SelectModule>> selects;
  };

}

#endif
