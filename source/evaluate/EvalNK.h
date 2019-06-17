/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  EvalNK.h
 *  @brief MABE Evaluation module for NK Landscapes
 */

#ifndef MABE_EVAL_NK_H
#define MABE_EVAL_NK_H

#include "../core/MABE.h"
#include "../core/Modules.h"
#include "../tools/NK.h"

namespace mabe {

  class EvalNK : public Module {
  private:
    size_t N;
    size_t K;
    NKLandscape landscape;
    emp::vector<emp::Ptr<mabe::Population>> eval_pops;

  public:
    EvalNK(size_t _N, size_t _K) : N(_N), K(_K) { }
    ~EvalNK() { }

    /// Add an additional population to evaluate.
    EvalNK & AddPopulation( mabe::Population & in_pop ) {
      eval_pops.push_back( &in_pop );
      return *this;
    }

    bool Setup(mabe::World & world) {
      // Setup the fitness landscape.
      landscape.Config(N, K, world.GetRandom());

      // If no populations have been identified for evaluation, assume pop 0.
      if (eval_pops.size() == 0) eval_pops.push_back( &(world.GetPopulation(0)) );

      return true;
    }

    bool Update() {
      return true;
    }
  };

}

#endif
