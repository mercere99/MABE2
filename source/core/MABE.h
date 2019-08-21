/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  MABE.h
 *  @brief Master controller object for a MABE run.
 */

#ifndef MABE_MABE_H
#define MABE_MABE_H

#include <string>

#include "base/Ptr.h"
#include "base/vector.h"
#include "config/command_line.h"
#include "control/Signal.h"
#include "tools/Random.h"
#include "tools/vector_utils.h"

#include "../config/Config.h"

#include "Module.h"
#include "Population.h"

namespace mabe {

  /// MABEBase sets up all population-manipulation functionality (nowhere else can alter a
  /// Population object).  As such, it guarantees that all manipulation calls ultimately
  /// come through the limited functions defined here.

  class MABEBase {
  protected:
    using Iterator = Population::Iterator;  ///< Use the same iterator as Population.

    template <typename... ARGS>
    struct ModVector : public emp::vector<emp::Ptr<Module>> {
      typedef void (Module::*ModMemFun)(ARGS...);
      ModMemFun fun;

      ModVector(ModMemFun _fun) : fun(_fun) { ; }

      template <typename... ARGS2>
      void Trigger(ARGS2 &&... args) {
        for (emp::Ptr<Module> mod_ptr : *this) {
          std::invoke(fun, *mod_ptr, std::forward<ARGS2>(args)...);
        }
      }
    };

    emp::vector<emp::Ptr<Module>> modules;  ///< Collection of ALL modules.

    // --- Track which modules need to have each signal type called on them. ---
    // BeforeUpdate(size_t update_ending)
    ModVector<size_t> before_update_mods;
    // OnUpdate(size_t new_update)
    ModVector<size_t> on_update_mods;
    // BeforeRepro(Iterator parent_pos) 
    ModVector<Iterator> before_repro_mods;
    // OnOffspringReady(Organism & offspring, Iterator parent_pos)
    ModVector<Organism &,Iterator> on_offspring_ready_mods;
    // OnInjectReady(Organism & inject_org)
    ModVector<Organism &> on_inject_ready_mods;
    // BeforePlacement(Organism & org, Iterator target_pos, Iterator parent_pos)
    ModVector<Organism &, Iterator, Iterator> before_placement_mods;
    // OnPlacement(Iterator placement_pos)
    ModVector<Iterator> on_placement_mods;
    // BeforeMutate(Organism & org)
    ModVector<Organism &> before_mutate_mods; // TO IMPLEMENT
    // OnMutate(Organism & org)
    ModVector<Organism &> on_mutate_mods; // TO IMPLEMENT
    // BeforeDeath(Iterator remove_pos)
    ModVector<Iterator> before_death_mods;
    // BeforeSwap(Iterator pos1, Iterator pos2)
    ModVector<Iterator,Iterator> before_swap_mods; // TO IMPLEMENT
    // OnSwap(Iterator pos1, Iterator pos2)
    ModVector<Iterator,Iterator> on_swap_mods; // TO IMPLEMENT
    // BeforePopResize(Population & pop, size_t new_size)
    ModVector<Population &, size_t> before_pop_resize_mods; // TO IMPLEMENT
    // OnPopResize(Population & pop, size_t old_size)
    ModVector<Population &, size_t> on_pop_resize_mods; // TO IMPLEMENT
    // OnNewOrgManager(OrganismManager & org_man)
    ModVector<OrganismManager &> on_new_org_manager_mods; // TO IMPLEMENT
    // BeforeExit()
    ModVector<> before_exit_mods; // TO IMPLEMENT
    // OnHelp()
    ModVector<> on_help_mods; // TO IMPLEMENT


    // Private constructor so that base class cannot be instantiated directly.
    MABEBase()
    : before_update_mods(&Module::BeforeUpdate)
    , on_update_mods(&Module::OnUpdate)
    , before_repro_mods(&Module::BeforeRepro)
    , on_offspring_ready_mods(&Module::OnOffspringReady)
    , on_inject_ready_mods(&Module::OnInjectReady)
    , before_placement_mods(&Module::BeforePlacement)
    , on_placement_mods(&Module::OnPlacement)
    , before_mutate_mods(&Module::BeforeMutate)
    , on_mutate_mods(&Module::OnMutate)
    , before_death_mods(&Module::BeforeDeath)
    , before_swap_mods(&Module::BeforeSwap)
    , on_swap_mods(&Module::OnSwap)
    , before_pop_resize_mods(&Module::BeforePopResize)
    , on_pop_resize_mods(&Module::OnPopResize)
    , on_new_org_manager_mods(&Module::OnNewOrgManager)
    , before_exit_mods(&Module::BeforeExit)
    , on_help_mods(&Module::OnHelp)
    { ;  }

  public:

    /// All insertions of organisms should come through AddOrgAt
    /// Must provide an org_ptr that is now own by the population.
    /// Must specify the pos in the population to perform the insertion.
    /// Must specify parent position if it exists (for data tracking); not used with inject.
    void AddOrgAt(emp::Ptr<Organism> org_ptr, Iterator pos, Iterator ppos=Iterator()) {
      before_placement_mods.Trigger(*org_ptr, pos, ppos);
      ClearOrgAt(pos);      // Clear out any organism already in this position.
      pos.SetOrg(org_ptr);  // Put the new organism in place.
      on_placement_mods.Trigger(pos);
    }

    /// All permanent deletion of organisms from a population should come through here.
    void ClearOrgAt(Iterator pos) {
      emp_assert(pos.IsValid());
      if (pos.IsEmpty()) return; // Nothing to remove!

      before_death_mods.Trigger(pos);
      pos.ExtractOrg().Delete();
    }

    /// All movement of organisms from one population position to another should come through here.
    void SwapOrgs(Iterator pos1, Iterator pos2) {
      // @CAO TRIGGER BEFORE SWAP SIGNAL!
      emp::Ptr<Organism> org1 = pos1.ExtractOrg();
      emp::Ptr<Organism> org2 = pos2.ExtractOrg();
      if (!org1->IsEmpty()) pos2.SetOrg(org1);
      if (!org2->IsEmpty()) pos1.SetOrg(org2);
      // @CAO TRIGGER ON SWAP SIGNAL!
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

  class MABE : public MABEBase {
  private:
    std::string VERSION = "0.0.1";

    emp::vector<Population> pops;           ///< Collection of populations.

    /// Collection of all organism types from all words.  Organism types have distinct
    /// names and can be manipulated as a whole.
    emp::unordered_map<std::string, emp::Ptr<OrganismManager>> org_managers;

    emp::Random random;              ///< Master random number generator
    int random_seed;                 ///< Random number seed.

    bool sync_pop = true;                   ///< Default to synchronous generations.
    size_t cur_pop = (size_t) -1;           ///< Which population are we currently working with?
    size_t update = 0;                      ///< How many times has Update() been called?
    emp::vector<std::string> errors;        ///< Log any errors that have occured.

    // --- Populaiton interaction ---
    using Iterator = Population::Iterator;  ///< Use the same iterator as Population.

    /// A birth placement function takes the new organism and an iterator pointing at a parent
    /// and returns an Interator indicating where that organism should place its offspring.
    using birth_pos_fun_t = std::function< Iterator(const Organism &, Iterator) >;
    birth_pos_fun_t birth_pos_fun;

    /// A birth placement function takes the injected organism and returns an Interator
    /// indicating where that organism should place its offspring.
    using inject_pos_fun_t = std::function< Iterator(const Organism &) >;
    inject_pos_fun_t inject_pos_fun;


    // --- Config information for command-line arguments ---
    struct ArgInfo {
      std::string name;   /// E.g.: "help" which would be called with "--help"
      std::string flag;   /// E.g.: "h" which would be called with -h
      std::string args;   /// Type of arguments needed: E.g.: "[filename...]"
      std::string desc;   /// E.g.: "Print the available command-line options for running mabe."

      /// Function to call when triggered.  Return bool to indicate if run should continue.
      using fun_t = std::function<bool(const emp::vector<std::string> &)>;
      fun_t action;

      ArgInfo(const std::string & _n, const std::string & _f, const std::string & _a,
              const std::string & _d, fun_t _action)
        : name(_n), flag(_f), args(_a), desc(_d), action(_action) { }
    };
    emp::vector<ArgInfo> arg_set;              ///< Map of arguments to the 
    emp::vector<std::string> args;             ///< Command-line arguments passed in.
    emp::vector<std::string> config_filenames; ///< Names of configuration files
    std::string gen_filename;                  ///< Name of output file to generate.
    Config config;                             ///< Configutation information for this run.

    // ----------- Helper Functions -----------

    void Exit() {
      // Cleanup all pointers.
      for (auto [name,org_manager] : org_managers) org_manager.Delete();
      org_managers.clear();

      // Forcibly exit.
      exit(0);
    }

    void ShowHelp() {
      std::cout << "MABE v" << VERSION << "\n"
                << "Usage: " << args[0] << " [options]\n"
                << "Options:\n";
      for (const auto & cur_arg : arg_set) {
        std::cout << "  " << cur_arg.flag << " " << cur_arg.args
                  << " : " << cur_arg.desc << " (or " << cur_arg.name << ")"
                  << std::endl;
      }
      std::cout << "Note: parameter order matters. Settings and files are applied in the order provided.\n";
      Exit();
    }

    void ShowModules() {
      std::cout << "MABE v" << VERSION << "\n"
                << "Active modules:\n";
      for (auto mod_ptr : modules) {
        std::cout << "  " << mod_ptr->GetName() << " : " << mod_ptr->GetDesc() << "\n";
      }          
      Exit();
    }

    template <typename... Ts>
    void AddError(Ts &&... args) {
      errors.push_back( emp::to_string( std::forward<Ts>(args)... ));
      std::cerr << "ERROR: " << errors.back() << std::endl;
    }
    void AddErrors(const emp::vector<std::string> & in_errors) {
      errors.insert(errors.end(), in_errors.begin(), in_errors.end());
    }

    void ProcessArgs();

    void Setup_Synchronisity();
    void Setup_Populations();
    void Setup_Modules();
    void Setup_Traits();

    void UpdateSignals();

  public:
    MABE(int argc, char* argv[]) : args(emp::cl::args_to_strings(argc, argv)) { ; }
    MABE(const MABE &) = delete;
    MABE(MABE &&) = delete;
    ~MABE() {
      for (auto x : modules) x.Delete();
      for (auto [name,org_manager] : org_managers) org_manager.Delete();
    }

    // --- Basic accessors ---

    emp::Random & GetRandom() { return random; }
    size_t GetUpdate() const noexcept { return update; }

    // --- Tools to setup runs ---
    void Setup() {
      SetupConfig(config.GetRootScope());  // Load all of the parameters needed by modules, etc.
      ProcessArgs();                       // Deal with command-line inputs.
      config.Load(config_filenames);       // Load files, if any.

      // If we are writing a file, do so and stop.
      if (gen_filename != "") { config.Write(gen_filename); Exit(); }

      Setup_Synchronisity();  // Should generations be synchronous or asynchronous?
      Setup_Populations();    // Give modules access to the correct populations.
      Setup_Modules();        // Run SetupModule() on each module; initialize placement if needed.
      Setup_Traits();         // Make sure module traits do not clash.

      UpdateSignals();        // Setup the appropriate modules to be linked with each signal.

      // Collect errors in any module.
      for (emp::Ptr<Module> mod_ptr : modules) {
        if (mod_ptr->HasErrors()) { AddErrors(mod_ptr->GetErrors()); }
      }
    }

    /// Update MABE a single step.
    void Update() {
      before_update_mods.Trigger(update);

      update++;
      std::cout << "Update: " << update << std::endl;

      // Run Update on all modules...
      on_update_mods.Trigger(update);

      // If we are running a synchronous reproduction, move the next generation to this one.
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
    }

    /// Update MABE the specified number of steps.
    void Update(size_t num_updates) {
      for (size_t ud = 0; ud < num_updates; ud++) {
        Update();
      }
    }


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

    void MoveOrg(Iterator from_pos, Iterator to_pos) {
      ClearOrgAt(to_pos);
      SwapOrgs(from_pos, to_pos);
    }

    void Inject(const Organism & org, size_t copy_count=1) {
      emp_assert(inject_pos_fun);
      for (size_t i = 0; i < copy_count; i++) {
        emp::Ptr<Organism> inject_org = org.Clone();
        on_inject_ready_mods.Trigger(*inject_org);
        Iterator pos = inject_pos_fun(*inject_org);
        if (pos.IsValid()) AddOrgAt( inject_org, pos);
        else {
          inject_org.Delete();
          AddError("Invalid position (pop=", pos.PopPtr(), "; pos=", pos.Pos(), "); failed to inject organism ", i, "!");
        }
      }
    }

    void Inject(const std::string & type_name, size_t copy_count=1) {
      const OrganismManager & org_manager = GetOrganismManager(type_name);
      for (size_t i = 0; i < copy_count; i++) {
        emp::Ptr<Organism> inject_org = org_manager.MakeOrganism(random);
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
      on_inject_ready_mods.Trigger(*inject_org);
      AddOrgAt( inject_org, pos);
    }

    // Give birth to (potentially) multiple offspring; return position of last placed.
    // Triggers 'before repro' signal on parent (once) and 'offspring ready' on each offspring.
    // Regular signal triggers occur in AddOrgAt.
    Iterator DoBirth(const Organism & org, Iterator ppos, size_t copy_count=1) {
      emp_assert(birth_pos_fun);           // Must have a value birth_pos_fun
      emp_assert(org.IsEmpty() == false);  // Empty cells cannot reproduce.
      before_repro_mods.Trigger(ppos);
      Iterator pos;                                        // Position of each offspring placed.
      for (size_t i = 0; i < copy_count; i++) {            // Loop through offspring, adding each
        emp::Ptr<Organism> new_org = org.Clone();          // Clone org to put copy in population
        on_offspring_ready_mods.Trigger(*new_org, ppos);   // Trigger modules with offspring ready
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
      MABEBase::ResizePop(pops[pop_id], new_size);
    }

    /// Resize a population while clearing all of the organisms in it.
    void EmptyPop(Population & pop, size_t new_size) {
      // Clean up any organisms in the population.
      for (Iterator it = pop.begin_alive(); it != pop.end(); ++it) {
        ClearOrgAt(it);
      }

      MABEBase::ResizePop(pop, new_size);
    }

    // --- Module Management ---

    int GetModuleID(const std::string & mod_name) const {
      return emp::FindEval(modules, [mod_name](const auto & m){ return m->GetName() == mod_name; });
    }

    const Module & GetModule(int id) const { return *modules[(size_t) id]; }
    Module & GetModule(int id) { return *modules[(size_t) id]; }

    template <typename MOD_T, typename... ARGS>
    MOD_T & AddModule(ARGS &&... args) {
      auto mod_ptr = emp::NewPtr<MOD_T>(*this, std::forward<ARGS>(args)...);
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


    // --- Deal with Organism Types ---

    OrganismManager & GetOrganismManager(const std::string & type_name) {
      emp_assert(emp::Has(org_managers, type_name)); // An org type must be created before base retrieved.
      return *(org_managers[type_name]);
    }

    template <typename ORG_TYPE_T>
    ORG_TYPE_T & AddOrganismManager(const std::string type_name) {
      emp_assert(emp::Has(org_managers, type_name) == false);
      auto new_type = emp::NewPtr<ORG_TYPE_T>(type_name);
      org_managers[type_name] = new_type;
      return *new_type;
    }

    /// Setup the configuration options for MABE.
    void SetupConfig(ConfigScope & config_scope) {
      config_scope.LinkVar(random_seed,
                           "random_seed",
                           "Seed for random number generator; use 0 to base on time.",
                           0).SetMin(0);

      // Call SetupConfig on each Population being used.
      auto & pops_scope = config_scope.AddScope("populations", "Specifications about the populations in this run.");
      for (auto & p : pops) p.SetupConfig(pops_scope);

      // Call the SetupConfig of module base classes (they will call the dervived version)
      auto & mods_scope = config_scope.AddScope("modules", "Specifications about the modules in this run.");
      for (auto m : modules) m->SetupConfig_Base(mods_scope);

      // Loop through organism types.
      auto & org_scope = config_scope.AddScope("org_managers", "Details about organisms types used in this runs.");
      for (auto o : org_managers) {
        auto & cur_scope = org_scope.AddScope(o.first, "Organism type");
        o.second->SetupConfig(cur_scope);
      }
    }
  };


  // ========================== OUT-OF-CLASS DEFINITIONS! ==========================

  void MABE::ProcessArgs() {
    arg_set.emplace_back("--filename", "-f", "[filename...] ", "Filenames of configuration settings",
      [this](const emp::vector<std::string> & in){ config_filenames = in; return true; } );
    arg_set.emplace_back("--generate", "-g", "[filename]    ", "Generate a new output file",
      [this](const emp::vector<std::string> & in) {
        if (in.size() != 1) {
          std::cout << "--generate must be followed by a single filename.\n";
          return false;
        }
        gen_filename = in[0];
        return true;
      });
    arg_set.emplace_back("--help", "-h", "              ", "Help; print command-line options for MABE",
      [this](const emp::vector<std::string> &){ ShowHelp(); return false; } );
    arg_set.emplace_back("--modules", "-m", "              ", "Module list",
      [this](const emp::vector<std::string> &){ ShowModules(); return false; } );
    arg_set.emplace_back("--set", "-s", "[param=value] ", "Set specified parameter",
      [this](const emp::vector<std::string> &){ emp_assert(false); return true; } );
    arg_set.emplace_back("--version", "-v", "              ", "Version ID of MABE",
      [this](const emp::vector<std::string> &){
        std::cout << "MABE v" << VERSION << "\n";
        return false;
      });

    // Scan through all input argument positions.
    bool show_help = false;
    for (size_t pos = 1; pos < args.size(); pos++) {
      // Match the input argument to the function to call.
      bool found = false;
      for (auto & cur_arg : arg_set) {
        // If we have a match...
        if (args[pos] == cur_arg.name || args[pos] == cur_arg.flag) {
          // ...collect all of the options associated with this match.
          emp::vector<std::string> option_args;
          // We want args until we run out or hit another option.
          while (++pos < args.size() && args[pos][0] != '-') {
            option_args.push_back(args[pos]);
          }

          // And call the function!
          cur_arg.action(option_args);
          found = true;
          break;            
        }
      }
      if (found == false) {
        std::cout << "Error: unknown command line argument '" << args[pos] << "'." << std::endl;
        show_help = true;
        break;
      }
    }

    if (show_help) ShowHelp();
  }

  void MABE::Setup_Synchronisity() {
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

  void MABE::Setup_Populations() {
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

  void MABE::Setup_Modules() {
    // Allow the user-defined module SetupModule() member functions run.  Goes through
    // the base class to record the current world.
    for (emp::Ptr<Module> mod_ptr : modules) mod_ptr->SetupModule();

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
  }

  void MABE::Setup_Traits() {

  }
  
  // Function to link signals to the modules that implment responses to those signals.
  void MABE::UpdateSignals() {
    before_update_mods.resize(0);
    on_update_mods.resize(0);
    before_repro_mods.resize(0);
    on_offspring_ready_mods.resize(0);
    on_inject_ready_mods.resize(0);
    before_placement_mods.resize(0);
    on_placement_mods.resize(0);
    before_mutate_mods.resize(0);
    on_mutate_mods.resize(0);
    before_death_mods.resize(0);
    before_swap_mods.resize(0);
    on_swap_mods.resize(0);
    before_pop_resize_mods.resize(0);
    on_pop_resize_mods.resize(0);
    on_new_org_manager_mods.resize(0);
    before_exit_mods.resize(0);
    on_help_mods.resize(0);

    for (emp::Ptr<Module> mod_ptr : modules) {
      if (mod_ptr->has_BeforeUpdate) before_update_mods.push_back(mod_ptr);
      if (mod_ptr->has_OnUpdate) on_update_mods.push_back(mod_ptr);
      if (mod_ptr->has_BeforeRepro) before_repro_mods.push_back(mod_ptr);
      if (mod_ptr->has_OnOffspringReady) on_offspring_ready_mods.push_back(mod_ptr);
      if (mod_ptr->has_OnInjectReady) on_inject_ready_mods.push_back(mod_ptr);
      if (mod_ptr->has_BeforePlacement) before_placement_mods.push_back(mod_ptr);
      if (mod_ptr->has_OnPlacement) on_placement_mods.push_back(mod_ptr);
      if (mod_ptr->has_BeforeMutate) before_mutate_mods.push_back(mod_ptr);
      if (mod_ptr->has_OnMutate) on_mutate_mods.push_back(mod_ptr);
      if (mod_ptr->has_BeforeDeath) before_death_mods.push_back(mod_ptr);
      if (mod_ptr->has_BeforeSwap) before_swap_mods.push_back(mod_ptr);
      if (mod_ptr->has_OnSwap) on_swap_mods.push_back(mod_ptr);
      if (mod_ptr->has_BeforePopResize) before_pop_resize_mods.push_back(mod_ptr);
      if (mod_ptr->has_OnPopResize) on_pop_resize_mods.push_back(mod_ptr);
      if (mod_ptr->has_OnNewOrgManager) on_new_org_manager_mods.push_back(mod_ptr);
      if (mod_ptr->has_BeforeExit) before_exit_mods.push_back(mod_ptr);
      if (mod_ptr->has_OnHelp) on_help_mods.push_back(mod_ptr);
    }
  }

}

#endif
