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

#include "../tools/NK.h"

namespace mabe {

  class EvalNK {
  private:
    size_t N;
    size_t K;
    NKLandscape landscape;

  public:
    EvalNK(size_t _N, size_t _K) : N(_N), K(_K) { }
    ~EvalNK() { }

    bool Setup(mabe::World & world) { landscape.Config(N, K, world.GetRandom()); return true; }

    bool Update() { return true; }
  };

}

#endif
