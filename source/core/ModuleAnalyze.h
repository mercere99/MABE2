/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ModuleAnalyze.h
 *  @brief Base class for MABE modules that analyze evolving populations.
 */

#ifndef MABE_MODULE_ANALYZE_H
#define MABE_MODULE_ANALYZE_H

#include <string>

#include "Module.h"
#include "World.h"

namespace mabe {

  class ModuleAnalyze : public Module {
  public:
    bool IsAnalyze() const { return false; }   

    void InitSetup(mabe::World & world) final { }
  };

}

#endif
