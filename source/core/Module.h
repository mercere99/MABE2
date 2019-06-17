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
#include "tools/reference_vector.h"
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

    // Internal, initial setup.
    virtual void InitSetup(mabe::World & world) = 0;

    virtual void Setup(mabe::World & world) { /* By default, assume no setup needed. */ }
    virtual void Update() { /* By default, do nothing at update. */ }
  };

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

    void InitSetup(mabe::World & world) final {
      // If no populations have been identified for evaluation, assume pop 0.
      if (populations.size() == 0) populations.push_back( world.GetPopulation(0) );
    }
  };

  class ModuleSelect : public Module {
  public:
    bool IsSelect() const { return false; }

    void InitSetup(mabe::World & world) final { }
  };

  class ModuleAnalyze : public Module {
  public:
    bool IsAnalyze() const { return false; }   

    void InitSetup(mabe::World & world) final { }
  };

}

#endif
