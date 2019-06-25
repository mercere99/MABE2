/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ModuleEvaluate.h
 *  @brief Base class for MABE modules that evaluate organisms.
 */

#ifndef MABE_MODULE_EVALUATE_H
#define MABE_MODULE_EVALUATE_H

#include <string>

#include "Module.h"
#include "World.h"

namespace mabe {

  class ModuleEvaluate : public Module {    
  protected:
    emp::reference_vector<mabe::Population> populations;  ///< Which population do we need to evaluate?

  public:
    bool IsEvaluate() const { return false; }

    /// Add an additional population to evaluate.
    ModuleEvaluate & AddPopulation( mabe::Population & in_pop ) {
      populations.push_back( in_pop );
      return *this;
    }

    void InternalSetup(mabe::World & world) final {
      // If no populations have been identified for evaluation, assume pop 0.
      if (populations.size() == 0) populations.push_back( world.GetPopulation(0) );
    }
  };

}

#endif
