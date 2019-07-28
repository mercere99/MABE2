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

  /// WorldBase sets up all population-manipulation functionality for World (nowhere else can
  /// alter a Population object).  As such, it guarantees that all manipulation calls ultimately
  /// come through the limited functions defined here.

  class WorldBase {
  protected:
    // --- All world signals go here to make sure they are called appropriately ---

  public:
    using Iterator = Population::Iterator;  ///< Use the same iterator as Population.

    /// All insertions of organisms should come through AddOrgAt
    /// Must provide an org_ptr that is now own by the population.
    /// Must specify the pos in the population to perform the insertion.
    /// Must specify parent position if it exists (for data tracking); not used with inject.
    void AddOrgAt(emp::Ptr<Organism> org_ptr, Iterator pos, Iterator ppos=Iterator()) {
      // @CAO: TRIGGER BEFORE PLACEMENT SIGNAL! Include both new organism and parent, if available.
      ClearOrgAt(pos);      // Clear out any organism already in this position.
      pos.SetOrg(org_ptr);  // Put the new organism in place.
      // @CAO: TRIGGER ON PLACEMENT SIGNAL!
    }

    /// All permanent deletion of organisms from a population should come through here.
    void ClearOrgAt(Iterator pos) {
      emp_assert(pos.IsValid());
      if (pos.IsEmpty()) return; // Nothing to remove!

      // @CAO: TRIGGER BEFORE DEATH SIGNAL!
      pos.ExtractOrg().Delete();
    }

    /// All movement of organisms from one population position to another should come through here.
    void MoveOrg(Iterator from_pos, Iterator to_pos) {
      emp_assert(from_pos.IsOccupied());
      // @CAO TRIGGER BEFORE MOVE SIGNAL!
      ClearOrgAt(to_pos);
      to_pos.SetOrg( from_pos.ExtractOrg() );
    }

    void ResizePop(Population & pop, size_t new_size) {
      // Clean up any organisms that may be getting deleted.
      const size_t old_size = pop.GetSize();                // Track the starting size.
      if (old_size == new_size) return;                     // If size isn't changing, we're done!

      // @CAO TRIGGER BEFORE POP RESIZE!

      for (size_t pos = new_size; pos < old_size; pos++) {  // Clear all orgs out of range.
        ClearOrgAt( Iterator(pop, pos) );
      }

      pop.Resize(new_size);                                 // Do the actual resize.

      // @CAO TRIGGER AFTER POP RESIZE!
    }

    Iterator PushEmpty(Population & pop) {
      // @CAO TRIGGER BEFORE POP RESIZE!
      return pop.PushEmpty();
      // @CAO TRIGGER AFTER POP RESIZE!
    }
  };

  class World : public WorldBase {
  private:
    std::string name;                       ///< Unique name for this world.

    //emp::vector<emp::Ptr<Population>> pops; ///< Set of populations in this world/
    emp::vector<Population> pops;           ///< Set of populations in this world.
    emp::vector<emp::Ptr<Module>> modules;  ///< Set of modules that configure this world.

    emp::Ptr<MABE> mabe_ptr;                ///< Pointer back to controlling MABE object.
    emp::Random & random;                   ///< Random number generator.
    size_t id;                              ///< What is the ID of this world in MABE?
    size_t cur_pop = (size_t) -1;           ///< Which population in this world is active?
    bool sync_pop = true;                   ///< Default to synchronous generations.

    size_t update = 0;                      ///< How many times has Update() been called?
    emp::vector<std::string> errors;        ///< Log any errors that have occured.


    // Organism placement functions

    using Iterator = Population::Iterator;  ///< Use the same iterator as Population.

    /// A birth placement function takes the new organism and an iterator pointing at a parent
    /// and returns an Interator indicating where that organism should place its offspring.
    using birth_pos_fun_t = std::function< Iterator(const Organism &, Iterator) >;
    birth_pos_fun_t birth_pos_fun;

    /// A birth placement function takes the injected organism and returns an Interator
    /// indicating where that organism should place its offspring.
    using inject_pos_fun_t = std::function< Iterator(const Organism &) >;
    inject_pos_fun_t inject_pos_fun;

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
      for (size_t pop_id = 0; pop_id < pops.size(); pop_id++) {
        const Population & from_pop = in_world.pops[pop_id];
        WorldBase::ResizePop(pops[pop_id], from_pop.GetSize());
        for (size_t org_id = 0; org_id < from_pop.GetSize(); org_id++) {
          if (from_pop.IsOccupied(org_id)) {
            AddOrgAt(from_pop[org_id].Clone(), Iterator(pops[pop_id], org_id));
          }
        }
      }
      for (size_t i = 0; i < modules.size(); i++) modules[i] = in_world.modules[i]->Clone();
    }
    World(World &&) = default;
    ~World() {
      for (auto x : modules) x.Delete();
    }

    // --- Basic Accessors ---

    const std::string & GetName() const noexcept { return name; }
    MABE & GetMABE() { return *mabe_ptr; }
    emp::Random & GetRandom() noexcept { return random; }
    size_t GetID() const noexcept { return id; }
    size_t GetUpdate() const noexcept { return update; }

    void SetName(const std::string & new_name) { name = new_name; }
    void SetID(size_t new_id) noexcept { id = new_id; }
    
    // --- Population Management ---

    size_t GetNumPopulations() { return pops.size(); }
    int GetPopID(const std::string & pop_name) const {
      return emp::FindEval(pops, [pop_name](const auto & p){ return p.GetName() == pop_name; });
    }
    const Population & GetPopulation(size_t id) const { return pops[id]; }
    Population & GetPopulation(size_t id) { return pops[id]; }

    /// New populaitons must be given a name and an optional size.
    Population & AddPopulation(const std::string & name, size_t pop_size=0) {
      cur_pop = (int) pops.size();
      pops.emplace_back( name, cur_pop, pop_size );
      return pops[cur_pop];
    }

    /// If GetPopulation() is called without an ID, return the current population or create one.
    Population & GetPopulation() {
      if (pops.size() == 0) {                // If we don't already have a population, add one!
        emp_assert(cur_pop == (size_t) -1);  // Current population should be default;
        AddPopulation("main");               // Default population is named main.
      }
      return pops[cur_pop];
    }

    void Inject(const Organism & org, size_t copy_count=1) {
      emp_assert(inject_pos_fun);
      for (size_t i = 0; i < copy_count; i++) {
        emp::Ptr<Organism> inject_org = org.Clone();
        // @CAO: Trigger Inject ready!
        Iterator pos = inject_pos_fun(*inject_org);
        if (pos.IsValid()) AddOrgAt( inject_org, pos);
        else {
          inject_org.Delete();
          AddError("Invalid position (pop=", pos.PopPtr(), "; pos=", pos.Pos(), "); failed to inject organism ", i, "!");
        }
      }
    }

    void InjectAt(const Organism & org, Iterator pos) {
      emp_assert(pos.IsValid());
      emp::Ptr<Organism> inject_org = org.Clone();
      // @CAO: Trigger Inject ready!
      AddOrgAt( inject_org, pos);
    }

    // Give birth to (potentially) multiple offspring; return position of last placed.
    // Triggers 'before repro' signal on parent (once) and 'offspring ready' on each offspring.
    // Regular signal triggers occur in AddOrgAt.
    Iterator DoBirth(const Organism & org, Iterator ppos, size_t copy_count=1) {
      emp_assert(birth_pos_fun);           // Must have a value birth_pos_fun
      emp_assert(org.IsEmpty() == false);  // Empty cells cannot reproduce.
      // @CAO Trigger before repro signal.
      Iterator pos;                                        // Position of each offspring placed.
      for (size_t i = 0; i < copy_count; i++) {            // Loop through offspring, adding each
        emp::Ptr<Organism> new_org = org.Clone();          // Clone org to put copy in population
        // @CAO Trigger offspring ready signal.
        pos = birth_pos_fun(*new_org, ppos);               // Determine location for offspring

        if (pos.IsValid()) AddOrgAt(new_org, pos, ppos);   // If placement pos is valid, do so!
        else new_org.Delete();                             // Otherwise delete the organism.
      }
      return pos;
    }

    /// A shortcut to DoBirth where only the parent position needs to be supplied.
    Iterator Replicate(Iterator ppos, size_t copy_count=1) {
      return DoBirth(*ppos, ppos, copy_count);
    }

    // Shortcut to resize a population by id.
    void ResizePop(size_t pop_id, size_t new_size) {
      emp_assert(pop_id < pops.size());
      WorldBase::ResizePop(pops[pop_id], new_size);
    }

    /// Resize a population while clearing all of the organisms in it.
    void EmptyPop(Population & pop, size_t new_size) {
      // Clean up any organisms in the population.
      for (Iterator it = pop.begin_alive(); it != pop.end(); ++it) {
        ClearOrgAt(it);
      }

      WorldBase::ResizePop(pop, new_size);
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

    // --- Built-in Population Management ---

    /// Set the placement function to put offspring at the end of a specified population.
    /// Organism replication and placement.
    void SetGrowthPlacement(size_t target_pop) {
      // Ignore both the organism and the parent; always insert at the end of the population.
      birth_pos_fun = [this,target_pop](const Organism &, Iterator) {
          return PushEmpty(pops[target_pop]);
        };
      inject_pos_fun = [this,target_pop](const Organism &) {
          return PushEmpty(pops[target_pop]);
        };
    }

    /// If we don't specific a population to place offspring in, assume they go in the current one.
    void SetGrowthPlacement() {
      if (sync_pop) SetGrowthPlacement(1);
      else SetGrowthPlacement(0);
    }


    // --- Configuration Controls ---

    World & OutputConfigSettings(std::ostream & os=std::cout, const std::string & prefix="") {
      os << prefix << name << " = {\n";

      // Print info about all populations in the world:
      os << prefix << "  populations = {\n";
      for (size_t i = 0; i < pops.size(); i++) {
        if (i) os << "\n";
        pops[i].OutputConfigSettings(os, prefix+"    ");
      }
      os << prefix << "  }\n\n";

      // Print info about all modules in the world:
      os << prefix << "  modules = {\n";
      for (size_t i = 0; i < modules.size(); i++) {
        if (i) os << "\n";
        modules[i]->OutputConfigSettings(os, prefix+"    ");
      }
      os << prefix << "  }" << std::endl;

      os << prefix << "}" << std::endl;

      return *this;
    }

    /// Setup the configuration options for this world.
    void SetupConfig(ConfigScope & config_scope) {
      // Call SetupConfig on each Population being used.
      auto & pops_scope = config_scope.AddScope("populations", "Specifications about the populations in this world.");
      for (auto p : pops) p->SetupConfig(pops_scope);

      // Call the SetupConfig of module base classes (they will call the dervived version)
      auto & mods_scope = config_scope.AddScope("modules", "Specifications about the modules in this world.");
      for (auto m : modules) m->SetupConfig_Base(mods_scope);
    }

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

      // If none of the modules setup the placement functions, do so now.
      if (!birth_pos_fun) {
        if (sync_pop) {
          std::cout << "Setting up SYNCHRONOUS reproduction." << std::endl;
          emp_assert(pops.size() >= 2);
          birth_pos_fun = [this](const Organism &, Iterator) {
              // Iterator it = pops[1].PushEmpty();
              // std::cout << "[[" << it.PopID() << ":" << it.Pos() << "]]" << std::endl;
              // return it;
              return PushEmpty(pops[1]);   // Synchronous pops reproduce into next generation.
            };
        } else {
          std::cout << "Setting up ASYNCHRONOUS reproduction." << std::endl;
          emp_assert(pops.size() >= 1);
          birth_pos_fun = [this](const Organism &, Iterator) {
              return PushEmpty(pops[0]);;   // Asynchronous offspring go into current population.
            };
        }
      }
      if (!inject_pos_fun) {
        inject_pos_fun = [this](const Organism &) {
            return PushEmpty(pops[0]);;
          };
      }

      // ############ STEP 4: Setup traits.
      Setup_Traits();

      // ############ STEP 5: Collect errors in any module.
      for (emp::Ptr<Module> mod_ptr : modules) {
        if (mod_ptr->HasErrors()) { AddErrors(mod_ptr->GetErrors()); }
      }

    }

    void Update() {
      // Run Update on all modules...
      for (emp::Ptr<Module> mod_ptr : modules) mod_ptr->Update(*this);

      // If we are running a synchronous world, move the next generation to this one.
      if (sync_pop) {
        Population & from_pop = pops[1];
        Population & to_pop = pops[0];

        // Clear out the current main population and resize.
        EmptyPop(to_pop, from_pop.GetSize());  

        // Move the next generation to the main population.
        Iterator it_to = to_pop.begin();
        for (Iterator it_from = from_pop.begin(); it_from != from_pop.end(); ++it_from, ++it_to) {
          if (it_from.IsOccupied()) MoveOrg(it_from, it_to);
        }

        // Clear out the next generation
        EmptyPop(from_pop, 0);
      }

      update++;
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
      // Determine how many populations this module needs.
      size_t min_pops = mod_ptr->GetMinPops();

      // Any additional populations should just be numbered.
      while (pops.size() < min_pops) AddPopulation(emp::to_string("pop", pops.size()-2));
    }

    // Leave main population as current.
    cur_pop = 0;
  }

  void World::Setup_Traits() {

  }
}

#endif
