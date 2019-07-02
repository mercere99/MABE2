/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  World.h
 *  @brief Controller for an individual World in MABE.
 * 
 *  A WORLD maintains one or more populations of organisms, along with a set of modules that
 *  control the rules by which those organisms evolve.  Modules types are:
 *  - Evaluate: Process organisms to measure phenotypic traits
 *  - Select: Use those traits to affect/determine each organism's reproductive success
 *  - Placement: Once an organism is selected for reproduction, place the offspring appropriately.
 *  - Analyze: Track aspects of evolution and perform measurements on populations.
 * 
 *  World will perform some automatic configuration and error-checking on its components.  We
 *  have a careful balance between intelligent default behaviors and requiring the user to
 *  specify any configurations that may be confusing.
 * 
 *  For example: Modules must specify which populations they are to act upon UNLESS they only
 *  work on the main population OR main and a "next generation" population for synchronous
 *  generations.
 * 
 *  Developer notes:
 *  - Populations are stored in a vector of *pointers* so that if the vector resizes, organisms
 *    will not change their position in memory.
 */

#ifndef MABE_WORLD_H
#define MABE_WORLD_H

#include <string>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/vector_utils.h"

#include "Population.h"
#include "Module.h"

namespace mabe {

  class MABE;

  class World {
  private:
    std::string name;

    emp::vector<emp::Ptr<Population>> pops;
    emp::vector<emp::Ptr<Module>> modules;

    emp::Ptr<MABE> mabe_ptr;
    emp::Random & random;
    size_t id;
    size_t cur_pop = (size_t) -1;

    emp::vector<std::string> errors;

    // Helper functions.
    void AddError(Ts &&... args) {
      errors.push_back( emp::to_string( std::forward<Ts>(args)... ));
    }
    void AddErrors(const emp::vector<std::string> & in_errors) {
      errors.insert(errors.end(), in_errors.begin(), in_errors.end());
    }

  public:
    World(const std::string & in_name, MABE & mabe, emp::Random & in_random, size_t in_id)
      : name(in_name), mabe_ptr(&mabe), random(in_random), id(in_id) { ; }
    World(const World & in_world) : pops(in_world.pops.size())
                                  , modules(in_world.modules.size())
                                  , mabe_ptr(in_world.mabe_ptr)
                                  , random(in_world.random)
                                  , id(in_world.id) {
      for (size_t i = 0; i < pops.size(); i++) pops[i] = emp::NewPtr<Population>(*in_world.pops[i]);
      for (size_t i = 0; i < modules.size(); i++) modules[i] = in_world.modules[i]->Clone();
    }
    World(World &&) = default;
    ~World() {
      for (auto x : pops) x.Delete();
      for (auto x : modules) x.Delete();
    }

    // --- Basic Accessors ---

    const std::string & GetName() const noexcept { return name; }
    MABE & GetMABE() { return *mabe_ptr; }
    emp::Random & GetRandom() noexcept { return random; }
    size_t GetID() const noexcept { return id; }

    void SetName(const std::string & new_name) { name = new_name; }
    void SetID(size_t new_id) noexcept { id = new_id; }
    
    // --- Population Management ---

    size_t GetNumPopulations() { return pops.size(); }
    int GetPopID(const std::string & pop_name) const {
      return emp::FindEval(pops, [pop_name](const auto & p){ return p->GetName() == pop_name; });
    }
    const Population & GetPopulation(size_t id) const { return *pops[id]; }
    Population & GetPopulation(size_t id) { return *pops[id]; }

    /// New populaitons must be given a name and an optional size.
    Population & AddPopulation(const std::string & name, size_t pop_size=0) {
      cur_pop = (int) pops.size();
      pops.push_back( emp::NewPtr<Population>(name, cur_pop, pop_size) );
      return *(pops[cur_pop]);
    }

    /// If GetPopulation() is called without an ID, return the current population or create one.
    Population & GetPopulation() {
      if (pops.size() == 0) {                // If we don't already have a population, add one!
        emp_assert(cur_pop == (size_t) -1);  // Current population should be default;
        AddPopulation("main");               // Default population is named main.
      }
      return *(pops[cur_pop]);
    }

    // --- Module Management ---

    int GetModuleID(const std::string & mod_name) const {
      return emp::FindEval(modules, [mod_name](const auto & m){ return m->GetName() == mod_name; });
    }

    const Module & GetModule(int id) const { return *modules[(size_t) id]; }
    Module & GetModule(int id) { return *modules[(size_t) id]; }

    template <typename MOD_T, typename... ARGS>
    MOD_T & AddModule(ARGS &&... args) {
      auto mod_ptr = emp::NewPtr<MOD_T>(std::forward<ARGS>(args)...);
      modules.push_back(mod_ptr);
      mod_ptr->InternalSetup(*this);
      return *mod_ptr;
    }

    // --- Basic Controls ---

    void Setup() {
      // Scan through the modules to make sure they are consistent and determine any automatic
      // configuration that still needs to be done.
      size_t required_pops = 0;
      for (emp::Ptr<Module> mod_ptr : modules) {
        // If this module requires more populations than we currently have, update!
        required_pops = std::max(required_pops, mod_ptr->GetRequiredPops());
      }

      // If no populations have been manually setup, make sure we have at least one.
      if (pops.size() == 0) AddPopulation("main");

      // Allow the user-defined module Setup() member functions run.
      for (auto x : modules) x->Setup(*this);

      // Final scan for additional errors in any module.
      for (emp::Ptr<Module> mod_ptr : modules) {
        if (mod_ptr->HasErrors()) { AddErrors(mod_ptr->GetErrors()); }
      }
    }

    void Update(size_t num_updates=1) {
      // Run Update on all modules...
      for (size_t ud = 0; ud < num_updates; ud++) {
        for (auto x : modules) x->Update();
      }
    }

  };

}

#endif
