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

#include <string>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/vector_utils.h"

#include "Population.h"
#include "Modules.h"

namespace mabe {

  class MABE;

  class World {
  private:
    std::string name;

    emp::vector<emp::Ptr<Population>> pops;
    emp::vector<emp::Ptr<ModuleEvaluate>> evals;
    emp::vector<emp::Ptr<ModuleSelect>> selects;

    emp::Ptr<MABE> mabe_ptr;
  public:
    World(const std::string & in_name, MABE & mabe) : name(in_name), mabe_ptr(&mabe) { ; }
    World(const World & in_world) : pops(in_world.pops.size())
                                  , evals(in_world.evals.size())
                                  , selects(in_world.selects.size())
                                  , mabe_ptr(in_world.mabe_ptr) {
      for (size_t i = 0; i < pops.size(); i++) pops[i] = emp::NewPtr<Population>(*in_world.pops[i]);
      for (size_t i = 0; i < evals.size(); i++) evals[i] = in_world.evals[i]->Clone();
      for (size_t i = 0; i < selects.size(); i++) selects[i] = in_world.selects[i]->Clone();
    }
    World(World &&) = default;
    ~World() {
      for (auto x : pops) x.Delete();
      for (auto x : evals) x.Delete();
      for (auto x : selects) x.Delete();
    }

    const std::string & GetName() const { return name; }
    
    int GetPopID(const std::string & pop_name) const {
      return emp::FindEval(pops, [pop_name](const auto & p){ return p->GetName() == pop_name; });
    }
    int GetEvalID(const std::string & e_name) const {
      return emp::FindEval(evals, [e_name](const auto & e){ return e->GetName() == e_name; });
    }
    int GetSelectID(const std::string & s_name) const {
      return emp::FindEval(selects, [s_name](const auto & s){ return s->GetName() == s_name; });
    }

    const Population & GetPopulation(int id) const { return *pops[(size_t) id]; }
    const ModuleEvaluate & GetModuleEvaluate(int id) const { return *evals[(size_t) id]; }
    const ModuleSelect & GetModuleSelect(int id) const { return *selects[(size_t) id]; }
    Population & GetPopulation(int id) { return *pops[(size_t) id]; }
    ModuleEvaluate & GetModuleEvaluate(int id) { return *evals[(size_t) id]; }
    ModuleSelect & GetModuleSelect(int id) { return *selects[(size_t) id]; }

    MABE & GetMABE() { return *mabe_ptr; }
  };

}

#endif
