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
#include "tools/Random.h"
#include "tools/vector_utils.h"

#include "../config/Config.h"

#include "Module.h"
#include "OrganismWrapper.h"
#include "Population.h"

namespace mabe {

  /// MABEBase sets up all population-manipulation functionality (nowhere else can alter a
  /// Population object).  As such, it guarantees that all manipulation calls ultimately
  /// come through the limited functions defined here.

  class MABEBase {
  protected:
    // --- All population signals go in here to make sure they are called appropriately ---

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

  class MABE : public MABEBase {
  private:
    std::string VERSION = "0.0.1";

    emp::vector<Population> pops;           ///< Collection of populations.
    emp::vector<emp::Ptr<Module>> modules;  ///< Collection of modules.

    /// Collection of all organism types from all words.  Organism types have distinct
    /// names and can be manipulated as a whole.
    emp::unordered_map<std::string, emp::Ptr<OrganismType>> org_types;

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
      for (auto [name,org_type] : org_types) org_type.Delete();
      org_types.clear();

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

    template <typename... Ts>
    void AddError(Ts &&... args) {
      errors.push_back( emp::to_string( std::forward<Ts>(args)... ));
      std::cerr << "ERROR: " << errors.back() << std::endl;
    }
    void AddErrors(const emp::vector<std::string> & in_errors) {
      errors.insert(errors.end(), in_errors.begin(), in_errors.end());
    }

  public:
    MABE(int argc, char* argv[]) : args(emp::cl::args_to_strings(argc, argv)) {
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
      arg_set.emplace_back("--set", "-s", "[param=value] ", "Set specified parameter",
        [this](const emp::vector<std::string> &){ emp_assert(false); return true; } );
      arg_set.emplace_back("--version", "-v", "              ", "Version ID of MABE",
        [this](const emp::vector<std::string> &){
          std::cout << "MABE v" << VERSION << "\n";
          return false;
        });
      // Command line options
      //  -p set parameter (name value)  (-s --set)
      //  -s write settings files        (-g --generate)
      //  -l creates population loader script (?)

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
    MABE(const MABE &) = delete;
    MABE(MABE &&) = delete;
    ~MABE() {
      for (auto x : modules) x.Delete();
      for (auto [name,org_type] : org_types) org_type.Delete();
    }

    // --- Basic accessors ---

    emp::Random & GetRandom() { return random; }
    size_t GetUpdate() const noexcept { return update; }

    // --- Tools to setup runs ---

    void Setup_Synchronisity();
    void Setup_Populations();
    void Setup_Traits();

    void Setup() {
      // Load all of the parameters needed by modules, etc.
      SetupConfig(config.GetRootScope());

      // If we are loading files, do so.
      config.Load(config_filenames);

      // If we are writing a file, do so.
      if (gen_filename != "") {
        config.Write(gen_filename);
        Exit();
      }

      // Now that parameters are loaded, setup all of the world for running.

      // STEP 1: Determine if updates should have synchronous or asynchronous generations.
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

    /// Update MABE the specified number of step.
    void Update(size_t num_updates=1) {
      for (size_t ud = 0; ud < num_updates; ud++) {
        std::cout << "Update: " << ud << std::endl;

        // Run Update on all modules...
        for (emp::Ptr<Module> mod_ptr : modules) mod_ptr->Update(*this);

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

        update++;

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


    // --- Deal with Organism Types ---

    OrganismType & GetOrganismType(const std::string & type_name) {
      emp_assert(emp::Has(org_types, type_name)); // An org type must be created before base retrieved.
      return *(org_types[type_name]);
    }

    template <typename ORG_T>
    OrganismWrapper<ORG_T> & GetFullOrganismType(const std::string type_name) {
      auto it = org_types.find(type_name);
      if (it == org_types.end()) {
        auto new_type = emp::NewPtr<OrganismWrapper<ORG_T>>(type_name);
        org_types[type_name] = new_type;
        return *new_type;
      }
      return *(it->second);
    }

    template <typename ORG_T>
    OrganismWrapper<ORG_T> & AddOrganismType(const std::string type_name) {
      emp_assert(emp::Has(org_types, type_name) == false);
      auto new_type = emp::NewPtr<OrganismWrapper<ORG_T>>(type_name);
      org_types[type_name] = new_type;
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
      auto & org_scope = config_scope.AddScope("org_types", "Details about organisms types used in this runs.");
      for (auto o : org_types) o.second->SetupConfig(org_scope);
    }
  };

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

  void MABE::Setup_Traits() {

  }
  
}

#endif
