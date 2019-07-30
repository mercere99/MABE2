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

#include "base/vector.h"
#include "config/command_line.h"
#include "tools/Random.h"
#include "tools/vector_utils.h"

#include "../config/Config.h"
#include "OrganismWrapper.h"
#include "Population.h"
#include "World.h"

namespace mabe {

  class MABE {
  private:
    /// Collect all world instances.  Each world will maintain its own environment
    /// (evaluate module), selection module, and populations of current organisms.
    emp::vector<emp::Ptr<mabe::World>> worlds;
    size_t cur_world = (size_t) -1;

    /// Collect all organism types from all words.  Organism types have distinct
    /// names and can be manipulated as a whole.
    emp::unordered_map<std::string, emp::Ptr<OrganismType>> org_types;

    emp::Random random;              ///< Master random number generator
    int random_seed;                 ///< Random number seed.

    // Config information for command-line arguments.
    struct ArgInfo {
      std::string name;   /// E.g.: "help" which would be called with "--help"
      std::string flag;   /// E.g.: "h" which would be called with -h
      std::string desc;   /// E.g.: "Print the available command-line options for running mabe."

      /// Function to call when triggered.
      using fun_t = std::function<bool(const emp::vector<std::string> &)>;
      fun_t action;

      ArgInfo(const std::string & _n, const std::string & _f, const std::string & _d, fun_t _a)
        : name(_n), flag(_f), desc(_d), action(_a) { }
    };
    emp::vector<ArgInfo> arg_set;              ///< Map of arguments to the 
    emp::vector<std::string> args;             ///< Command-line arguments passed in.
    emp::vector<std::string> config_filenames; ///< Names of configuration files
    bool exit_now = false;                     ///< Do we need to exit?
    Config config;                             ///< Configutation information for this run.

    void ShowHelp() {
      std::cout << "Usage: " << args[0] << " [options]\n"
                << "Options:\n";
      for (const auto & cur_arg : arg_set) {
        std::cout << "  " << cur_arg.flag << ", " << cur_arg.name
                  << " : " << cur_arg.desc << std::endl;
      }
      exit_now = true;
    }

    /// ShowHelp varient that takes args for command-line call.
    bool ShowHelp(const emp::vector<std::string> &) { ShowHelp(); return true; }

  public:
    MABE(int argc, char* argv[]) : args(emp::cl::args_to_strings(argc, argv)) {
      arg_set.emplace_back("--filename", "-f", "Set the names of all configuration files",
        [this](const emp::vector<std::string> & in){ config_filenames = in; return true; } );
      // Command line options
      //  -f filename (for config files)
      //  -p set parameter (name value)
      //  -s write settings files
      //  -l creates population loader script
      //  -v provides version id

      // Scan through all input argument positions.
      bool show_help = false;
      for (size_t pos = 1; pos < args.size(); pos++) {
        // Match the input argument to the function to call.
        bool found = false;
        for (auto & cur_arg : arg_set) {
          // If we have a match...
          if (args[0] == cur_arg.name || args[0] == cur_arg.flag) {
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
      for (auto x : worlds) x.Delete();
      for (auto [name,org_type] : org_types) org_type.Delete();
    }

    // --- Basic accessors ---

    emp::Random & GetRandom() { return random; }

    // --- Basic Controls ---

    void Setup() {
      if (exit_now) return;
      SetupConfig(config.GetRootScope());
      config.Load(config_filenames);
//        config.Write();
      for (emp::Ptr<mabe::World> w : worlds) w->Setup();
    }

    /// By default, update all worlds the specified numebr of updates.
    void Update(size_t num_updates=1) {
      if (exit_now) return;
      for (size_t ud = 0; ud < num_updates; ud++) {
        std::cout << "Update: " << ud << std::endl;
        for (emp::Ptr<mabe::World> w : worlds) w->Update();
      }
    }

    // --- Deal with World management ---

    size_t GetNumWorlds() const { return worlds.size(); }

    /// Add a new world with a specific name, make it current, and return its ID.
    mabe::World & AddWorld(const std::string & name) {
      cur_world = (int) worlds.size();
      worlds.push_back( emp::NewPtr<mabe::World>(name, *this, random, cur_world) );
      return *(worlds[cur_world]);
    }

    /// Retrieve a world by its ID.
    mabe::World & GetWorld(int id) {
      emp_assert(id >= 0 && id < (int) worlds.size());
      return *(worlds[(size_t) id]);
    }

    /// With no arguments, GetWorld() returns the current world or creates
    /// a new world if none have been created yet.
    mabe::World & GetWorld() {
      if (worlds.size() == 0) {
        emp_assert(cur_world == (size_t) -1);
        AddWorld("main_world");
      }
      return GetWorld(cur_world);
    }

    /// Get the ID of a world with a specific name.
    int GetWorldID(const std::string & name) const {
      return emp::FindEval(worlds, [name](auto w){ return w->GetName() == name; });
    }

    /// If GetWorld() is called with a world name, look up its ID and return it.
    mabe::World & GetWorld(const std::string & name) {
      const int world_id = GetWorldID(name);
      emp_assert(world_id >= 0 && world_id < (int) worlds.size(),
                 "Unknown world name; perhaps you need to create it firsts?",
                 name);
      return GetWorld(world_id);
    }

    // --- Deal with Organism Type ---

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

    // --- Deal with actual organisms ---

    // Inject a specific organism - pass on to current world.
    void InjectOrganism(const Organism & org, size_t copy_count=1) {
      if (exit_now) return;
      GetWorld().Inject(org, copy_count);
    }


    // --- Forward module management to current world ---
    template <typename MOD_T, typename... ARGS>
    auto & AddModule(ARGS &&... args) {
      return GetWorld().AddModule<MOD_T>(std::forward<ARGS>(args)...);
    }


    /// Setup the configuration options for MABE.
    void SetupConfig(ConfigScope & config_scope) {
      config_scope.LinkVar(random_seed,
                           "random_seed",
                           "Seed for random number generator; use 0 to base on time.",
                           0).SetMin(0);
      auto & org_scope = config_scope.AddScope("org_types", "Details about organisms types used in this runs.");
      for (auto o : org_types) o.second->SetupConfig(org_scope);

      // Loop through Worlds.
      auto & worlds_scope = config_scope.AddScope("worlds", "Worlds created for this MABE run.");
      for (auto w : worlds) w->SetupConfig(worlds_scope);      
    }
  };

}

#endif
