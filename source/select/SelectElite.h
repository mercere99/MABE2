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
#include "../core/ModuleSelect.h"

#include "tools/reference_vector.h"

namespace mabe {

  class SelectElite : public ModuleSelect {
  private:
    std::string trait;

  public:
    SelectElite(const std::string & in_trait) : trait(in_trait) { }
    ~SelectElite() { }

    void Setup(mabe::World & world) {
    }

    void Update() {
    }
  };

}

#endif