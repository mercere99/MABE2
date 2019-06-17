/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ModuleSelect.h
 *  @brief Base class for MABE modules that select organisms to move on to the next generation.
 */

#ifndef MABE_MODULE_SELECT_H
#define MABE_MODULE_SELECT_H

#include <string>

#include "Module.h"
#include "World.h"

namespace mabe {

  class ModuleSelect : public Module {
  public:
    bool IsSelect() const { return false; }

    void InitSetup(mabe::World & world) final { }
  };

}

#endif
