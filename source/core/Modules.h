/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Modules.h
 *  @brief Concepts that define all of the modules in MABE.
 */

#ifndef MABE_MODULES_H
#define MABE_MODULES_H

#include <string>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/vector_utils.h"

namespace mabe {

  class MABE;
  class World;

  class Module {
  private:
    std::string name;

  public:
    Module() : name("") { ; }
    Module(const Module &) = default;
    Module(Module &&) = default;
    virtual ~Module() { ; }

    const std::string & GetName() const { return name; }

    virtual emp::Ptr<Module> Clone() { return nullptr; }

    virtual bool IsEvaluate() const { return false; }
    virtual bool IsSelect() const { return false; }
    virtual bool IsAnalyze() const { return false; }

    virtual bool Setup(mabe::World & world) { /* By default, assume no setup needed. */ return false; }
    virtual bool Update() { /* By default, do nothing at update. */ return false; }
  };

  class ModuleEvaluate : public Module {    
  public:
    bool IsEvaluate() const { return false; }
  };

  class ModuleSelect : public Module {
  public:
    bool IsSelect() const { return false; }
  };

  class ModuleAnalyze : public Module {
  public:
    bool IsAnalyze() const { return false; }   
  };

}

#endif
