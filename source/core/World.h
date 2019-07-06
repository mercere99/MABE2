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
    std::string name;                       ///< Unique name for this world.

    emp::vector<emp::Ptr<Population>> pops; ///< Set of populations in this world/
    emp::vector<emp::Ptr<Module>> modules;  ///< Set of modules that configure this world.

    emp::Ptr<MABE> mabe_ptr;                ///< Pointer back to controlling MABE object.
    emp::Random & random;                   ///< Random number generator.
    size_t id;                              ///< What is the ID of this world in MABE?
    size_t cur_pop = (size_t) -1;           ///< Which population in this world is active?

    emp::vector<std::string> errors;        ///< Log any errors that have occured.

    bool sync_pop = true;                   ///< Default to a synchronous population.

    // Helper functions.
    template <typename... Ts>
    void AddError(Ts &&... args) {
      errors.push_back( emp::to_string( std::forward<Ts>(args)... ));
      std::cerr << "ERROR: " << errors.back() << std::endl;
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

    void InjectOrganism(const Organism & org) {
      GetPopulation().Inject(org);
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
      return *mod_ptr;
    }

    // --- Configuration Controls ---

    void Setup_Synchronisity();
    void Setup_Populations();
    void Setup_Traits();

    void Setup() {
      // STEP 1: Determine if world updates should have synchronous or asynchronous generations.
      Setup_Synchronisity();

      // STEP 2: Make sure modules have access to the correct number of populations.
      Setup_Populations();

      // ############ STEP 3: Run Setup() on all modules.
      // Allow the user-defined module Setup() member functions run.
      for (emp::Ptr<Module> mod_ptr : modules) mod_ptr->Setup(*this);

      // ############ STEP 4: Setup traits.
      Setup_Traits();

      // ############ STEP 5: Collect errors in any module.
      for (emp::Ptr<Module> mod_ptr : modules) {
        if (mod_ptr->HasErrors()) { AddErrors(mod_ptr->GetErrors()); }
      }

    }

    void Update(size_t num_updates=1) {
      // Run Update on all modules...
      for (size_t ud = 0; ud < num_updates; ud++) {
        for (emp::Ptr<Module> mod_ptr : modules) mod_ptr->Update();
      }
    }

  };

  void World::Setup_Synchronisity() {
    emp::Ptr<Module> async_req_mod = nullptr;
    emp::Ptr<Module> sync_req_mod = nullptr;
    size_t prefer_async = 0;
    size_t prefer_sync = 0;

    for (emp::Ptr<Module> mod_ptr : modules) {
      switch (mod_ptr->rep_type) {
        case Module::ReplicationType::NO_PREFERENCE:
          break;
        case Module::ReplicationType::REQUIRE_ASYNC:
          if (sync_req_mod) {
            AddError("Module ", sync_req_mod->name, " requires synchronous generations, but module ",
                      mod_ptr->name, " requires asynchronous.");
          }
          async_req_mod = mod_ptr;
          sync_pop = false;
          break;
        case Module::ReplicationType::DEFAULT_ASYNC:
          prefer_async++;
          break;
        case Module::ReplicationType::DEFAULT_SYNC:
          prefer_sync++;
          break;
        case Module::ReplicationType::REQUIRE_SYNC:
          if (async_req_mod) {
            AddError("Module ", async_req_mod->name, " requires asynchronous generations, but module ",
                      mod_ptr->name, " requires synchronous.");
          }
          sync_req_mod = mod_ptr;
          sync_pop = true;
          break;
      }
    }
    // If we don't have any requirements, go with the preference!
    if (!async_req_mod && !sync_req_mod) sync_pop = prefer_sync >= prefer_async;
  }

  void World::Setup_Populations() {
    // If no populations have been setup by the user, build a "main" population.
    if (pops.size() == 0) AddPopulation("main_pop");

    // If we are synchronous, also create a "next" population.
    if (pops.size() == 1 && sync_pop) AddPopulation("next_pop");

    // Now loop through the modules and make sure all populations are assigned.
    for (emp::Ptr<Module> mod_ptr : modules) {
      // Determine how many populations this module has, and how many it needs.
      size_t cur_pops = mod_ptr->GetNumPops();
      size_t min_pops = mod_ptr->GetMinPops();
      size_t max_pops = mod_ptr->GetMaxPops();

      // If the number of populations is already in range, we're done with this module.
      if (cur_pops >= min_pops && cur_pops <= max_pops) continue;

      // If there are too many populations, this is clearly an error.
      if (cur_pops > max_pops) {
        AddError(cur_pops, " populations set in module '", mod_ptr->GetName(),
                  "', but only ", max_pops, " allowed.");
        continue;
      }

      // If we are not allowed to automatically assign population -or- if there are some
      // populations specified, but not enough, don't try to guess, just print error.
      if (mod_ptr->DefaultPopsOK() == false || (cur_pops > 0 && cur_pops < min_pops)) {
        AddError(cur_pops, " populations set in module '", mod_ptr->GetName(),
                  "', but ", min_pops, " required.");
        continue;
      }

      // Any additional populations should just be numbered.
      while (pops.size() < min_pops) AddPopulation(emp::to_string("pop", pops.size()-2));

      // Assign populations to this module.
      for (size_t i = 0; i < min_pops; i++) {
        mod_ptr->UsePopulation(*(pops[i]));
      }
    }

    // Leave main population as current.
    cur_pop = 0;
  }

  void World::Setup_Traits() {

  }
}

#endif
