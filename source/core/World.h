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
    emp::vector<emp::Ptr<Module>> modules;

    emp::Ptr<MABE> mabe_ptr;
    emp::Random & random;
  public:
    World(const std::string & in_name, MABE & mabe, emp::Random & in_random)
      : name(in_name), mabe_ptr(&mabe), random(in_random) { ; }
    World(const World & in_world) : pops(in_world.pops.size())
                                  , modules(in_world.modules.size())
                                  , mabe_ptr(in_world.mabe_ptr)
                                  , random(in_world.random) {
      for (size_t i = 0; i < pops.size(); i++) pops[i] = emp::NewPtr<Population>(*in_world.pops[i]);
      for (size_t i = 0; i < modules.size(); i++) modules[i] = in_world.modules[i]->Clone();
    }
    World(World &&) = default;
    ~World() {
      for (auto x : pops) x.Delete();
      for (auto x : modules) x.Delete();
    }

    // --- Basic Accessors ---

    const std::string & GetName() const { return name; }
    MABE & GetMABE() { return *mabe_ptr; }
    emp::Random & GetRandom() { return random; }
    
    // --- Population Management ---

    int GetPopID(const std::string & pop_name) const {
      return emp::FindEval(pops, [pop_name](const auto & p){ return p->GetName() == pop_name; });
    }
    const Population & GetPopulation(int id) const { return *pops[(size_t) id]; }
    Population & GetPopulation(int id) { return *pops[(size_t) id]; }

    // --- Module Management ---

    int GetModuleID(const std::string & mod_name) const {
      return emp::FindEval(modules, [mod_name](const auto & m){ return m->GetName() == mod_name; });
    }

    const Module & GetModule(int id) const { return *modules[(size_t) id]; }
    Module & GetModule(int id) { return *modules[(size_t) id]; }

    template <typename MOD_T, typename... ARGS>
    auto & AddModule(ARGS &&... args) {
      auto mod_ptr = emp::NewPtr<MOD_T>(std::forward<ARGS>(args)...);
      modules.push_back(mod_ptr);
      return *mod_ptr;
    }

    // --- Basic Controls ---

    void Setup() {
      for (auto x : modules) x->Setup(*this);
    }

  };

}

#endif
