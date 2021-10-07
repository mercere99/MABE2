/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  MABE.hpp
 *  @brief Master controller object for a MABE run.
 *
 *  This is the master MABE controller object that hooks together all of the modules and provides
 *  the interface for them to interact.
 *
 *  MABE allows modules to interact via a set of SIGNALS that they can listen for by overriding
 *  specific base class functions.  See Module.h for a full list of available signals.
 */

#ifndef MABE_MABE_H
#define MABE_MABE_H

#include <limits>
#include <string>
#include <sstream>

#include "emp/base/array.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/config/command_line.hpp"
#include "emp/control/Signal.hpp"
#include "emp/data/DataMap.hpp"
#include "emp/data/DataMapParser.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/io/StreamManager.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"

#include "../config/Config.hpp"

#include "Collection.hpp"
#include "data_collect.hpp"
#include "ErrorManager.hpp"
#include "MABEBase.hpp"
#include "ModuleBase.hpp"
#include "Population.hpp"
#include "SigListener.hpp"
#include "TraitManager.hpp"

namespace mabe {

  ///  @brief The main MABE controller class
  ///
  ///  The MABE controller class manages interactions between all modules,
  ///  ensures that all need components are present at startup, and triggers
  ///  signals as needed.
  ///
  ///  Note that this class is derived from MABEBase, which handles all population
  ///  manipulation and signal management.

  class MABE : public MABEBase {
  private:
    const std::string VERSION = "0.0.1";

    // --- Variables to handle configuration, initialization, and error reporting ---

    bool verbose = false;        ///< Should we output extra information during setup?
    bool show_help = false;      ///< Should we show "help" before exiting?
    bool exit_now = false;       ///< Do we need to immediately clean up and exit the run?
    ErrorManager error_man;      ///< Object to manage warnings and errors.
    emp::StreamManager files;    ///< Track all of the file streams used in MABE.

    // Setup helper types.
    using trait_equation_t = std::function<double(emp::DataMap &)>;
    using trait_summary_t = std::function<std::string(const Collection &)>;

    // Setup a cache for functions used to collect data for files.
    std::unordered_map<std::string, emp::vector<trait_summary_t>> file_fun_cache;

    /// Populations used; generated in the configuration file.
    emp::vector< emp::Ptr<Population> > pops;

    /// Organism pointer to use for all empty cells.
    emp::Ptr<Organism> empty_org = nullptr;

    TraitManager<ModuleBase> trait_man;  ///< Manages which modules are allowed to use each trait.

    /// Trait information to be stored on each organism.  Tracks the name, type, and current
    /// value of all traits that modules associate with organisms.
    emp::DataMap org_data_map;

    emp::DataMapParser dm_parser;      ///< Parser to process functions on a data map.

    emp::Random random;                ///< Master random number generator
    size_t cur_pop_id = (size_t) -1;   ///< Which population is currently active?
    size_t update = 0;                 ///< How many times has Update() been called?


    // --- Config information for command-line arguments ---
    struct ArgInfo {
      std::string name;                ///< E.g.: "help" which would be called with "--help"
      std::string flag;                ///< E.g.: "h" which would be called with -h
      std::string args;                ///< Type of arguments needed: E.g.: "[filename...]"
      std::string desc;                ///< E.g.: "Print available command-line options."

      /// Function to call when triggered.
      using fun_t = std::function<void(const emp::vector<std::string> &)>;
      fun_t action;

      ArgInfo(const std::string & _n, const std::string & _f, const std::string & _a,
              const std::string & _d, fun_t _action)
        : name(_n), flag(_f), args(_a), desc(_d), action(_action) { }
    };

    emp::vector<ArgInfo> arg_set;              ///< Info about valid command-line arguments.
    emp::vector<std::string> args;             ///< Command-line arguments passed in.
    emp::vector<std::string> config_filenames; ///< Names of configuration files to load.
    emp::vector<std::string> config_settings;  ///< Additional config commands to run.
    std::string gen_filename;                  ///< Name of output file to generate.
    Config config;                             ///< Configuration information for this run.
    emp::Ptr<ConfigEntry_Scope> cur_scope;     ///< Which config scope are we currently using?


    // ----------- Helper Functions -----------

    /// Print information on how to run the software.
    void ShowHelp();

    /// List all of the available modules included in the current compilation.
    void ShowModules();

    /// Ask evaluation modules to trace the execution of the provided organism.
    void TraceEval(Organism & org, std::ostream & os) { trace_eval_sig.Trigger(org, os); }

    /// Process all of the arguments that were passed in on the command line.
    void ProcessArgs();

    // -- Helper functions to be called inside of Setup() --

    /// Run SetupModule() method on each module we've loaded.
    void Setup_Modules();

    /// Load organism traits that modules need to read or write and test for conflicts.
    void Setup_Traits();

    /// Link signals to the modules that implement responses to those signals.
    void UpdateSignals();

    /// Find any instances of ${X} and eval the X.
    std::string Preprocess(const std::string & in_string);

    /// Setup a function as deprecated so we can phase it out.
    void Deprecate(const std::string & old_name, const std::string & new_name);

  public:
    MABE(int argc, char* argv[]);  ///< MABE command-line constructor.
    MABE(const MABE &) = delete;
    MABE(MABE &&) = delete;
    ~MABE() {
      if (empty_org) empty_org.Delete();                           // Delete empty_org ptr.
      for (auto x : modules) x.Delete();                           // Delete all modules.
      for (auto x : pops) x.Delete();                              // Delete all populations.
    }

    // --- Basic accessors ---
    emp::Random & GetRandom() { return random; }
    size_t GetUpdate() const noexcept { return update; }
    bool GetVerbose() const { return verbose; }
    mabe::ErrorManager & GetErrorManager() { return error_man; }

    /// Output args if (and only if) we are in verbose mode.
    template <typename... Ts> void Verbose(Ts &&... args) {
      if (verbose) std::cout << emp::to_string(std::forward<Ts>(args)...) << std::endl;
    }

    // --- Tools to setup runs ---
    bool Setup();

    /// Setup an organism as a placeholder for all "empty" positions in the population.
    template <typename EMPTY_MANAGER_T> void SetupEmpty();

    /// Update MABE a single time step.
    void Update();

    /// Update MABE a specified number of time steps.
    void DoRun(size_t num_updates);

    // -- World Structure --

    OrgPosition FindBirthPosition(Organism & offspring, OrgPosition ppos, Population & pop) {
      return do_place_birth_sig.FindPosition(offspring, ppos, pop);
    }
    OrgPosition FindInjectPosition(Organism & new_org, Population & pop) {
      return do_place_inject_sig.FindPosition(new_org, pop);
    }
    OrgPosition FindNeighbor(OrgPosition pos) {
      return do_find_neighbor_sig.FindPosition(pos);
    }


    // --- Population Management ---

    size_t GetNumPopulations() const { return pops.size(); }
    int GetPopID(std::string_view pop_name) const {
      return emp::FindEval(pops, [pop_name](const auto & p){ return p->GetName() == pop_name; });
    }
    const Population & GetPopulation(size_t id) const { return *pops[id]; }
    Population & GetPopulation(size_t id) { return *pops[id]; }

    /// New populations must be given a name and an optional size.
    Population & AddPopulation(const std::string & name, size_t pop_size=0);

    /// If GetPopulation() is called without an ID, return the current population or create one.
    Population & GetPopulation();

    /// Move an organism from one position to another; kill anything that previously occupied
    /// the target position.
    void MoveOrg(OrgPosition from_pos, OrgPosition to_pos) {
      ClearOrgAt(to_pos);
      SwapOrgs(from_pos, to_pos);
    }

    /// Inject a copy of the provided organism and return the position it was placed in;
    /// if more than one is added, return the position of the final injection.
    OrgPosition Inject(const Organism & org, Population & pop, size_t copy_count=1);

    /// Inject this specific instance of an organism and turn over the pointer to be managed
    /// by MABE.  Teturn the position the organism was placed in.
    OrgPosition InjectInstance(emp::Ptr<Organism> org_ptr, Population & pop);
    

    /// Add an organsim of a specified type to the world (provide the type name and the
    /// MABE controller will create instances of it.)  Returns the position of the last
    /// organism placed.
    OrgPosition Inject(const std::string & type_name, Population & pop, size_t copy_count=1);

    /// Add an organism of a specified type and population (provide names of both and they
    /// will be properly setup.)
    OrgPosition Inject(const std::string & type_name, 
                       const std::string & pop_name,
                       size_t copy_count=1);

    /// Inject a copy of the provided organism at a specified position.
    void InjectAt(const Organism & org, OrgPosition pos) {
      emp_assert(pos.IsValid());
      emp::Ptr<Organism> inject_org = org.CloneOrganism();
      on_inject_ready_sig.Trigger(*inject_org, GetPopulation(pos.PopID()));
      AddOrgAt( inject_org, pos);
    }

    /// Give birth to one or more offspring; return position of last placed.
    /// Triggers 'before repro' signal on parent (once) and 'offspring ready' on each offspring.
    /// Regular signal triggers occur in AddOrgAt.
    OrgPosition DoBirth(const Organism & org,
                        OrgPosition ppos,
                        Population & target_pop,
                        size_t birth_count=1,
                        bool do_mutations=true);

    OrgPosition DoBirth(const Organism & org,
                        OrgPosition ppos,
                        OrgPosition target_pos,
                        bool do_mutations=true);


    /// A shortcut to DoBirth where only the parent position needs to be supplied.
    OrgPosition Replicate(OrgPosition ppos, Population & target_pop,
                          size_t birth_count=1, bool do_mutations=true) {
      return DoBirth(*ppos, ppos, target_pop, birth_count, do_mutations);
    }

    /// Resize a population while clearing all of the organisms in it.
    void EmptyPop(Population & pop, size_t new_size);

    /// Return a ramdom position from a desginated population.
    OrgPosition GetRandomPos(Population & pop) {
      emp_assert(pop.GetSize() > 0);
      return pop.IteratorAt( random.GetUInt(pop.GetSize()) );
    }

    /// Return a ramdom position from the population with the specified id.
    OrgPosition GetRandomPos(size_t pop_id) { return GetRandomPos(GetPopulation(pop_id)); }

    /// Return a ramdom position from a desginated population with a living organism in it.
    OrgPosition GetRandomOrgPos(Population & pop);

    /// Return a ramdom position of a living organism from the population with the specified id.
    OrgPosition GetRandomOrgPos(size_t pop_id) { return GetRandomOrgPos(GetPopulation(pop_id)); }


    // --- Collection Management ---

    std::string ToString(const mabe::Collection & collect) const {
      return collect.ToString();
    }

    Collection ToCollection(const std::string & load_str);

    Collection GetAlivePopulation(size_t id) {
      Collection col(GetPopulation(id));
      col.RemoveEmpty();
      return col;
    }


    // --- Module Management ---

    /// Get the unique id of a module with the specified name.
    int GetModuleID(const std::string & mod_name) const {
      return emp::FindEval(modules, [mod_name](const auto & m){ return m->GetName() == mod_name; });
    }

    /// Get a reference to a module with the specified ID.
    const ModuleBase & GetModule(int id) const { return *modules[(size_t) id]; }
    ModuleBase & GetModule(int id) { return *modules[(size_t) id]; }

    /// Get a reference to a module with the specified name.
    const ModuleBase & GetModule(const std::string & mod_name) const {
      return *modules[(size_t) GetModuleID(mod_name)];
    }
    ModuleBase & GetModule(const std::string & mod_name) {
      return *modules[(size_t) GetModuleID(mod_name)];
    }

    /// Add a new module of the specified type.
    template <typename MOD_T, typename... ARGS>
    MOD_T & AddModule(ARGS &&... args) {
      auto new_mod = emp::NewPtr<MOD_T>(*this, std::forward<ARGS>(args)...);
      modules.push_back(new_mod);
      return *new_mod;
    }

    // --- Deal with Organism TRAITS ---
    TraitManager<ModuleBase> & GetTraitManager() { return trait_man; }

    /// Build a function to scan a single data map and run the provided equation on the
    /// enties in there, returning the result.

    trait_equation_t BuildTraitEquation(const std::string & equation) {
      return dm_parser.BuildMathFunction(org_data_map, equation);
    }

    /// Scan a provided equation and return the names of all traits used in that equation.

    const std::set<std::string> & GetEquationTraits(const std::string & equation) {
      return dm_parser.GetNamesUsed(equation);
    }

    /// Build a function to scan a collection of organisms, reading the value for the given
    /// trait_name from each, aggregating those values based on the trait_filter and returning
    /// the result as a string.

    trait_summary_t BuildTraitSummary(const std::string & trait_name, std::string trait_filter);

    // Handler for printing trait data
    void OutputTraitData(std::ostream & os,
                         Collection target_collect,
                         std::string format,
                         bool print_headers=false);

    // --- Manage configuration scope ---

    /// Access to the current configuration scope.
    ConfigEntry_Scope & GetCurScope() { return *cur_scope; }

    /// Add a new scope under the current one.
    ConfigEntry_Scope & AddScope(const std::string & name, const std::string & desc) {
      cur_scope = &(cur_scope->AddScope(name, desc));
      return *cur_scope;
    }

    /// Move up one level of scope.
    ConfigEntry_Scope & LeaveScope() { return *(cur_scope = cur_scope->GetScope()); }

    /// Return to the root scope.
    ConfigEntry_Scope & ResetScope() { return *(cur_scope = &(config.GetRootScope())); }

    /// Setup the configuration options for MABE, including for each module.
    void SetupConfig();

    /// Sanity checks for debugging
    bool OK();


    // Checks for which modules are currently being triggered.
    using mod_ptr_t = emp::Ptr<ModuleBase>;
    bool BeforeUpdate_IsTriggered(mod_ptr_t mod) { return before_update_sig.cur_mod == mod; };
    bool OnUpdate_IsTriggered(mod_ptr_t mod) { return on_update_sig.cur_mod == mod; };
    bool BeforeRepro_IsTriggered(mod_ptr_t mod) { return before_repro_sig.cur_mod == mod; };
    bool OnOffspringReady_IsTriggered(mod_ptr_t mod) { return on_offspring_ready_sig.cur_mod == mod; };
    bool OnInjectReady_IsTriggered(mod_ptr_t mod) { return on_inject_ready_sig.cur_mod == mod; };
    bool BeforePlacement_IsTriggered(mod_ptr_t mod) { return before_placement_sig.cur_mod == mod; };
    bool OnPlacement_IsTriggered(mod_ptr_t mod) { return on_placement_sig.cur_mod == mod; };
    bool BeforeMutate_IsTriggered(mod_ptr_t mod) { return before_mutate_sig.cur_mod == mod; };
    bool OnMutate_IsTriggered(mod_ptr_t mod) { return on_mutate_sig.cur_mod == mod; };
    bool BeforeDeath_IsTriggered(mod_ptr_t mod) { return before_death_sig.cur_mod == mod; };
    bool BeforeSwap_IsTriggered(mod_ptr_t mod) { return before_swap_sig.cur_mod == mod; };
    bool OnSwap_IsTriggered(mod_ptr_t mod) { return on_swap_sig.cur_mod == mod; };
    bool BeforePopResize_IsTriggered(mod_ptr_t mod) { return before_pop_resize_sig.cur_mod == mod; };
    bool OnPopResize_IsTriggered(mod_ptr_t mod) { return on_pop_resize_sig.cur_mod == mod; };
    bool OnError_IsTriggered(mod_ptr_t mod) { return on_error_sig.cur_mod == mod; };
    bool OnWarning_IsTriggered(mod_ptr_t mod) { return on_warning_sig.cur_mod == mod; };
    bool BeforeExit_IsTriggered(mod_ptr_t mod) { return before_exit_sig.cur_mod == mod; };
    bool TraceEval_IsTriggered(mod_ptr_t mod) { return trace_eval_sig.cur_mod == mod; };
    bool OnHelp_IsTriggered(mod_ptr_t mod) { return on_help_sig.cur_mod == mod; };
    bool DoPlaceBirth_IsTriggered(mod_ptr_t mod) { return do_place_birth_sig.cur_mod == mod; };
    bool DoPlaceInject_IsTriggered(mod_ptr_t mod) { return do_place_inject_sig.cur_mod == mod; };
    bool DoFindNeighbor_IsTriggered(mod_ptr_t mod) { return do_find_neighbor_sig.cur_mod == mod; };
  };


  // ========================== OUT-OF-CLASS DEFINITIONS! ==========================

  // ---------------- PRIVATE MEMBER FUNCTIONS -----------------

  /// Print information on how to run the software.
  void MABE::ShowHelp() {
    std::cout << "MABE v" << VERSION << "\n"
              << "Usage: " << args[0] << " [options]\n"
              << "Options:\n";
    for (const auto & cur_arg : arg_set) {
      std::cout << "  " << cur_arg.flag << " " << cur_arg.args
                << " : " << cur_arg.desc << " (or " << cur_arg.name << ")"
                << std::endl;
    }
    on_help_sig.Trigger();
    std::cout << "Note: Settings and files are applied in the order provided.\n";
    exit_now = true;
  }

  /// List all of the available modules included in the current compilation.
  void MABE::ShowModules() {
    std::cout << "MABE v" << VERSION << "\n"
              << "Active modules:\n";
    // for (auto mod_ptr : modules) {
    //   std::cout << "  " << mod_ptr->GetName() << " : " << mod_ptr->GetDesc() << "\n";
    // }          
    std::cout << "Available modules:\n";
    for (auto & info : GetModuleInfo()) {
      std::cout << "  " << info.name << " : " << info.desc << "\n";
    }          
    exit_now = true;;
  }

  void MABE::ProcessArgs() {
    arg_set.emplace_back("--filename", "-f", "[filename...] ", "Filenames of configuration settings",
      [this](const emp::vector<std::string> & in){ config_filenames = in; } );
    arg_set.emplace_back("--generate", "-g", "[filename]    ", "Generate a new output file",
      [this](const emp::vector<std::string> & in) {
        if (in.size() != 1) {
          std::cout << "'--generate' must be followed by a single filename.\n";
          exit_now = true;
        } else {
          // MABE Config files should be generated FROM a *.gen file, typically creating a *.mabe
          // file.  If output file is *.gen assume an error. (for now; override should be allowed)
          if (in[0].size() > 4 && in[0].substr(in[0].size()-4) == ".gen") {
            error_man.AddError("Error: generated file ", in[0], " not allowed to be *.gen; typically should end in *.mabe.");
            exit_now = true;
          }
          else gen_filename = in[0];
        }
      });
    arg_set.emplace_back("--help", "-h", "              ", "Help; print command-line options for MABE",
      [this](const emp::vector<std::string> &){ show_help = true; } );
    arg_set.emplace_back("--modules", "-m", "              ", "Module list",
      [this](const emp::vector<std::string> &){ ShowModules(); } );
    arg_set.emplace_back("--set", "-s", "[param=value] ", "Set specified parameter",
      [this](const emp::vector<std::string> & in){
        emp::Append(config_settings, in);
        config_settings.push_back(";"); // Extra semi-colon so not needed on command line.
      });
    arg_set.emplace_back("--version", "-v", "              ", "Version ID of MABE",
      [this](const emp::vector<std::string> &){
        std::cout << "MABE v" << VERSION << "\n";
        exit_now = true;
      });
    arg_set.emplace_back("--verbose", "-+", "              ", "Output extra setup info",
      [this](const emp::vector<std::string> &){ verbose = true; } );

    // Scan through all input argument positions.
    for (size_t pos = 1; pos < args.size(); pos++) {
      // Match the input argument to the function to call.
      bool found = false;
      for (auto & cur_arg : arg_set) {
        // If we have a match...
        if (args[pos] == cur_arg.name || args[pos] == cur_arg.flag) {
          // ...collect all of the options associated with this match.
          emp::vector<std::string> option_args;
          // We want args until we run out or hit another option.
          while (pos+1 < args.size() && args[pos+1][0] != '-') {
            option_args.push_back(args[++pos]);
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

  /// As part of the main Setup(), run SetupModule() method on each module we've loaded.
  void MABE::Setup_Modules() {
    // Allow the user-defined module SetupModule() member functions run.  These are
    // typically used for any internal setup needed by modules are the configuration is
    // complete.
    for (emp::Ptr<ModuleBase> mod_ptr : modules) mod_ptr->SetupModule();
  }

  /// As part of the main Setup(), load in all of the organism traits that modules need to
  /// read or write and make sure that there aren't any conflicts.
  void MABE::Setup_Traits() {
    Verbose("Analyzing configuration of ", trait_man.GetSize(), " traits.");

    trait_man.Verify(verbose);            // Make sure modules are accessing traits consistently
    trait_man.RegisterAll(org_data_map);  // Load in all of the traits to the DataMap
    org_data_map.LockLayout();            // Freeze the data map into its current state

    // Alert modules (especially org managers) to the final set of traits.
    for (emp::Ptr<ModuleBase> mod_ptr : modules) {
      mod_ptr->SetupDataMap(org_data_map);
    }
  }

  /// Link signals to the modules that implement responses to those signals.
  void MABE::UpdateSignals() {
    // Clear all module vectors.
    for (auto modv : sig_ptrs) modv->resize(0);

    // Loop through each module to update its signals.
    for (emp::Ptr<ModuleBase> mod_ptr : modules) {
      // If a module is deactivated, don't use it's signals.
      if (mod_ptr->_active == false) continue;

      // For the current module, loop through all of the signals.
      for (size_t sig_id = 0; sig_id < sig_ptrs.size(); sig_id++) {
        if (mod_ptr->has_signal[sig_id]) sig_ptrs[sig_id]->push_back(mod_ptr);
      }
    }

    // Now that we have scanned the signals, we can turn off the rescan flag.
    rescan_signals = false;
  }

  /// Find any instances of ${X} and eval the X.
  std::string MABE::Preprocess(const std::string & in_string) {
    std::string out_string = in_string;

    // Seek out instances of "${" to indicate the start of pre-processing.
    for (size_t i = 0; i < out_string.size(); ++i) {
      if (out_string[i] != '$') continue;   // Replacement tag must start with a '$'.
      if (out_string.size() <= i+2) break;  // Not enough room for a replacement tag.
      if (out_string[i+1] == '$') {         // Compress two $$ into on $
        out_string.erase(i,1);
        continue;
      }
      if (out_string[i+1] != '{') continue; // Eval must be surrounded by braces.

      // If we made it this far, we have a starting match!
      size_t end_pos = emp::find_paren_match(out_string, i+1, '{', '}', false);
      if (end_pos == i+1) return out_string;  // No end brace found!  @CAO -- exception here?
      const std::string replacement_text =
        config.Eval(emp::view_string_range(out_string, i+2, end_pos));
      out_string.replace(i, end_pos-i+1, replacement_text);

      i += replacement_text.size(); // Continue from the end point...
    }

    return out_string;
  }

  void MABE::Deprecate(const std::string & old_name, const std::string & new_name) {
    std::function<int(const emp::vector<emp::Ptr<ConfigEntry>> &)> dep_fun =
      [this,old_name,new_name](const emp::vector<emp::Ptr<ConfigEntry>> &){
        std::cerr << "Function '" << old_name << "' deprecated; use '" << new_name << "'\n";
        exit_now = true;
        return 0;      
      };

    config.AddFunction(old_name, dep_fun, std::string("Deprecated.  Use: ") + new_name);
  }

  // ---------------- PUBLIC MEMBER FUNCTIONS -----------------


  MABE::MABE(int argc, char* argv[])
    : error_man( [this](const std::string & msg){ on_error_sig.Trigger(msg); },
                 [this](const std::string & msg){ on_warning_sig.Trigger(msg); } )
    , trait_man(error_man)
    , args(emp::cl::args_to_strings(argc, argv))
    , cur_scope(&(config.GetRootScope()))
  {
    // Setup "Population" as a type in the config file.
    std::function<ConfigType &(const std::string &)> pop_init_fun =
      [this](const std::string & name) -> ConfigType & {
        return AddPopulation(name);
      };
    config.AddType("Population", "Collection of organisms", pop_init_fun);

    // Setup all known modules as available types in the config file.
    for (auto & mod : GetModuleInfo()) {
      std::function<ConfigType &(const std::string &)> mod_init_fun =
        [this,&mod](const std::string & name) -> ConfigType & {
          return mod.init_fun(*this,name);
        };
      config.AddType(mod.name, mod.desc, mod_init_fun);
    }


    // ------ DEPRECATED FUNCTION NAMES ------
    Deprecate("exit", "EXIT");
    Deprecate("inject", "INJECT");
    Deprecate("print", "PRINT");

    // Add other built-in functions to the config file.

    // 'EXIT' terminates a run gracefully.
    std::function<int()> exit_fun = [this](){ exit_now = true; return 0; };
    config.AddFunction("EXIT", exit_fun, "Exit from this MABE run.");


    // 'INJECT' allows a user to add an organism to a population.
    std::function<int(const std::string &, const std::string &, size_t)> inject_fun =
      [this](const std::string & org_type_name, const std::string & pop_name, size_t count) {
        Inject(org_type_name, pop_name, count);
        return 0;
      };
    config.AddFunction("INJECT", inject_fun,
      "Inject organisms into a population (args: org_name, pop_name, org_count).");


    std::function<std::string(const std::string &)> preprocess_fun =
      [this](const std::string & str) { return Preprocess(str); };
    config.AddFunction("PP", preprocess_fun, "Preprocess a string (replacing any ${...} with result.)");


    // @CAO Should be a method on a Population or Collection, not called by name.
    std::function<int(const std::string &)> pop_size_fun =
      [this](const std::string & target) { return ToCollection(target).GetSize(); };
    config.AddFunction("SIZE", pop_size_fun, "Return the size of the target population.");


    // 'WRITE' will collect data and write it to a file.
    files.SetOutputDefaultFile();  // Stream manager should default to files for output.
    std::function<int(const std::string &, const std::string &, const std::string &)> write_fun =
      [this](const std::string & filename, const std::string & collection, std::string format) {
        const bool file_exists = files.Has(filename);           // Is file is already setup?
        std::ostream & file = files.GetOutputStream(filename);  // File to write to.
        OutputTraitData(file, ToCollection(collection), format, !file_exists);
        return 0;
      };
    config.AddFunction("WRITE", write_fun,
      "Write the provided trait-based data to file; args: filename, collection, format.");


    // --- ORGANISM-BASED FUNCTIONS ---

    std::function<int(const std::string &, const std::string &, double)> trace_eval_fun =
      [this](const std::string & filename, const std::string & target, double id) {
        Collection c = ToCollection(target);                   // Collection with organisms
        Organism & org = c.At((size_t) id);                    // Specific organism to analyze.
        std::ostream & file = files.GetOutputStream(filename); // File to write to.
        TraceEval(org, file);
        return 0;
      };
    config.AddFunction("TRACE_EVAL", trace_eval_fun, "Return the size of the target population.");

    // --- TRAIT-BASED FUNCTIONS ---

    std::function<std::string(const std::string &, std::string)> trait_string_fun =
      [this](const std::string & target, std::string trait_filter) {
        std::string trait_name = emp::string_pop(trait_filter,':');
        auto fun = BuildTraitSummary(trait_name, trait_filter);
        return fun( ToCollection(target) );
      };
    config.AddFunction("TRAIT_STRING", trait_string_fun, "Collect information about a specified trait.");

    std::function<double(const std::string &, std::string)> trait_value_fun =
      [this](const std::string & target, std::string trait_filter) {
        std::string trait_name = emp::string_pop(trait_filter,':');
        auto fun = BuildTraitSummary(trait_name, trait_filter);
        return emp::from_string<double>(fun( ToCollection(target) ));
      };
    config.AddFunction("TRAIT_VALUE", trait_value_fun, "Collect information about a specified trait.");

    // std::function<double(const std::string &, const std::string &)> trait_mean_fun =
    //   [this](const std::string & target, const std::string & trait) {
    //     if constexpr (std::is_arithmetic_v<DATA_T>) {
    //       double total = 0.0;
    //       size_t count = 0;
    //       for (const auto & entry : container) {
    //         total += (double) get_fun(entry);
    //         count++;
    //       }
    //       return emp::to_string( total / count );
    //     }
    //     return 0.0; // @CAO: or Nan?
    //   };
    // config.AddFunction("trait_mean", trait_mean_fun, "Return the size of the target population.");


    // Add in built-in event triggers; these are used to indicate when events should happen.
    config.AddEventType("start");   // Triggered at the beginning of a run.
    config.AddEventType("update");  // Tested every update.
  }

  bool MABE::Setup() {
    SetupConfig();                   // Load all of the parameters needed by modules, etc.
    ProcessArgs();                   // Deal with command-line inputs.

    // Sometimes command-line arguments will require an immediate exit (such as after '--help')
    if (exit_now) return false;

    // If configuration filenames have been specified, load each of them in order.
    if (config_filenames.size()) {
      std::cout << "Loading file(s): " << emp::to_quoted_list(config_filenames) << std::endl;
      config.Load(config_filenames);   // Load files
    }

    if (config_settings.size()) {
      std::cout << "Loading command-line settings." << std::endl;
      config.LoadStatements(config_settings, "command-line settings");      
    }

    // If we are writing a file, do so and then exit.
    if (gen_filename != "") {
      std::cout << "Generating file '" << gen_filename << "'." << std::endl;
      config.Write(gen_filename);
      exit_now = true;
    }

    // If any of the inital flags triggered an 'exit_now', do so.
    if (exit_now) return false;

    // Allow traits to be linked.
    trait_man.Unlock();

    Setup_Modules();        // Run SetupModule() on each module for linking traits or other setup.
    Setup_Traits();         // Make sure module traits do not clash.

    UpdateSignals();        // Setup the appropriate modules to be linked with each signal.

    error_man.Activate();

    // Only return success if there were no errors.
    return (error_man.GetNumErrors() == 0);
  }

  /// Update MABE a single step.
  void MABE::Update() {
    // When in debug mode, check the integrity of MABE each update.
    emp_assert(OK(), update);

    // If informaiton on any of the signals has changed, update them.
    if (rescan_signals) UpdateSignals();

    // Signal that a new update is about to begin.
    before_update_sig.Trigger(update);

    // Increment 'update' to start new update.
    update++;

    // Run Update on all modules...
    on_update_sig.Trigger(update);

    // Trigger any events that are supposed to occur in config at this update.
    config.UpdateEventValue("update", update);
  }


  /// Setup an organism as a placeholder for all "empty" positions in the population.
  template <typename EMPTY_MANAGER_T>
  void MABE::SetupEmpty() {
    if (empty_org) empty_org.Delete();  // If we already have an empty organism, replace it.
    auto & empty_manager =
      AddModule<EMPTY_MANAGER_T>("EmptyOrg", "Manager for all 'empty' organisms in any population.");
    empty_manager.SetBuiltIn();         // Don't write the empty manager to config.

    empty_org = empty_manager.template Make<Organism>();
  }

  /// Update MABE a specified number of time steps.
  void MABE::DoRun(size_t num_updates) {
    config.TriggerEvents("start");
    for (size_t ud = 0; ud < num_updates && !exit_now; ud++) {
      Update();
    }
    before_exit_sig.Trigger();
  }

  /// New populations must be given a name and an optional size.
  Population & MABE::AddPopulation(const std::string & name, size_t pop_size) {
    cur_pop_id = (int) pops.size();                                   // Set new pop to "current"
    emp::Ptr<Population> new_pop =
      emp::NewPtr<Population>(name, cur_pop_id, pop_size, empty_org); // Create new population.
    pops.push_back(new_pop);                                          // Record new population.
    return *new_pop;                                                  // Return new population.
  }

  /// If GetPopulation() is called without an ID, return the current population or create one.
  Population & MABE::GetPopulation() {
    if (pops.size() == 0) {                   // If we don't have a population, add one!
      emp_assert(cur_pop_id == (size_t) -1);  // Current population should now be default;
      AddPopulation("main");                  // Default population is named main.
    }
    return *pops[cur_pop_id];
  }

  /// Inject a copy of the provided organism and return the position it was placed in;
  /// if more than one is added, return the position of the final injection.
  OrgPosition MABE::Inject(const Organism & org, Population & pop, size_t copy_count) {
    emp_assert(org.GetDataMap().SameLayout(org_data_map));
    OrgPosition pos;
    for (size_t i = 0; i < copy_count; i++) {
      emp::Ptr<Organism> inject_org = org.CloneOrganism();
      on_inject_ready_sig.Trigger(*inject_org, pop);
      pos = FindInjectPosition(*inject_org, pop);
      if (pos.IsValid()) {
        AddOrgAt( inject_org, pos);
      } else {
        inject_org.Delete();          
        error_man.AddError("Invalid position; failed to inject organism ", i, "!");
      }
    }
    return pos;
  }

  /// Inject this specific instance of an organism and turn over the pointer to be managed
  /// by MABE.  Teturn the position the organism was placed in.
  OrgPosition MABE::InjectInstance(emp::Ptr<Organism> org_ptr, Population & pop) {
    emp_assert(org_ptr->GetDataMap().SameLayout(org_data_map));
    on_inject_ready_sig.Trigger(*org_ptr, pop);
    OrgPosition pos = FindInjectPosition(*org_ptr, pop);
    if (pos.IsValid()) AddOrgAt( org_ptr, pos);
    else {
      org_ptr.Delete();          
      error_man.AddError("Invalid position; failed to inject organism!");
    }
    return pos;
  }
  

  /// Add an organsim of a specified type to the world (provide the type name and the
  /// MABE controller will create instances of it.)  Returns the position of the last
  /// organism placed.
  OrgPosition MABE::Inject(const std::string & type_name, Population & pop, size_t copy_count) {
    Verbose("Injecting ", copy_count, " orgs of type '", type_name,
            "' into population ", pop.GetID());

    auto & org_manager = GetModule(type_name);    // Look up type of organism.
    OrgPosition pos;                              // Place to save injection position.
    for (size_t i = 0; i < copy_count; i++) {     // Loop through, injecting each instance.
      auto org_ptr = org_manager.Make<Organism>(random);  // ...Build an org of this type.
      pos = InjectInstance(org_ptr, pop);         // ...Inject it into the population.
    }
    return pos;                                   // Return last position injected.
  }

  /// Add an organism of a specified type and population (provide names of both and they
  /// will be properly setup.)
  OrgPosition MABE::Inject(const std::string & type_name, 
                      const std::string & pop_name,
                      size_t copy_count) {      
    int pop_id = GetPopID(pop_name);
    if (pop_id == -1) {
      error_man.AddError("Invalid population name used in inject '", pop_name, "'.");        
    }
    Population & pop = GetPopulation(pop_id);
    OrgPosition pos = Inject(type_name, pop, copy_count);  // Inject a copy of the organism.
    return pos;                                           // Return last position injected.
  }

  /// Give birth to one or more offspring; return position of last placed.
  /// Triggers 'before repro' signal on parent (once) and 'offspring ready' on each offspring.
  /// Regular signal triggers occur in AddOrgAt.
  OrgPosition MABE::DoBirth(const Organism & org,
                      OrgPosition ppos,
                      Population & target_pop,
                      size_t birth_count,
                      bool do_mutations) {
    emp_assert(org.IsEmpty() == false);  // Empty cells cannot reproduce.
    before_repro_sig.Trigger(ppos);
    OrgPosition pos;                                      // Position of each offspring placed.
    emp::Ptr<Organism> new_org;
    for (size_t i = 0; i < birth_count; i++) {            // Loop through offspring, adding each
      new_org = do_mutations ? org.MakeOffspringOrganism(random) : org.CloneOrganism();

      // Alert modules that offspring is ready, then find its birth position.
      on_offspring_ready_sig.Trigger(*new_org, ppos, target_pop);
      pos = FindBirthPosition(*new_org, ppos, target_pop);

      // If this placement is valid, do so.  Otherwise delete the organism.
      if (pos.IsValid()) AddOrgAt(new_org, pos, ppos);
      else new_org.Delete();
    }
    return pos;
  }

  OrgPosition MABE::DoBirth(const Organism & org,
                      OrgPosition ppos,
                      OrgPosition target_pos,
                      bool do_mutations) {
    emp_assert(org.IsEmpty() == false);  // Empty cells cannot reproduce.
    emp_assert(target_pos.IsValid());    // Target positions must already be valid.

    before_repro_sig.Trigger(ppos);
    emp::Ptr<Organism> new_org = do_mutations ? org.MakeOffspringOrganism(random) : org.CloneOrganism();
    on_offspring_ready_sig.Trigger(*new_org, ppos, target_pos.Pop());

    AddOrgAt(new_org, target_pos, ppos);

    return target_pos;
  }


  /// Resize a population while clearing all of the organisms in it.
  void MABE::EmptyPop(Population & pop, size_t new_size) {
    // Clean up any organisms in the population.
    for (PopIterator pos = pop.begin(); pos != pop.end(); ++pos) {
      ClearOrgAt(pos);
    }

    MABEBase::ResizePop(pop, new_size);
  }


  /// Return a ramdom position from a desginated population with a living organism in it.
  OrgPosition MABE::GetRandomOrgPos(Population & pop) {
    emp_assert(pop.GetNumOrgs() > 0, "GetRandomOrgPos cannot be called if there are no orgs.");
    // @CAO: Something better to do in a sparse population?
    OrgPosition pos = GetRandomPos(pop);
    while (pos.IsEmpty()) pos = GetRandomPos(pop);
    return pos;
  }


  // --- Collection Management ---

  Collection MABE::ToCollection(const std::string & load_str) {
    Collection out;
    auto slices = emp::view_slices(load_str, ',');
    for (auto name : slices) {
      int pop_id = GetPopID(name);
      if (pop_id == -1) error_man.AddError("Unknown population: ", name);
      else out.Insert(GetPopulation(pop_id));
    }
    return out;
  }


  /// Build a function to scan a collection of organisms, reading the value for the given
  /// trait_name from each, aggregating those values based on the trait_filter and returning
  /// the result as a string.
  ///
  ///  trait_filter option are:
  ///   <none>      : Default to the value of the trait for the first organism in the collection.
  ///   [ID]        : Value of this trait for the organism at the given index of the collection.
  ///   [OP][VALUE] : Count how often this value has the [OP] relationship with [VALUE].
  ///                  [OP] can be ==, !=, <, >, <=, or >=
  ///                  [VALUE] can be any numeric value
  ///   [OP][TRAIT] : Count how often this trait has the [OP] relationship with [TRAIT]
  ///                  [OP] can be ==, !=, <, >, <=, or >=
  ///                  [TRAIT] can be any other trait name
  ///   unique      : Return the number of distinct value for this trait (alias="richness").
  ///   mode        : Return the most common value in this collection (aliases="dom","dominant").
  ///   min         : Return the smallest value of this trait present.
  ///   max         : Return the largest value of this trait present.
  ///   ave         : Return the average value of this trait (alias="mean").
  ///   median      : Return the median value of this trait.
  ///   variance    : Return the variance of this trait.
  ///   stddev      : Return the standard deviation of this trait.
  ///   sum         : Return the summation of all values of this trait (alias="total")
  ///   entropy     : Return the Shannon entropy of this value.
  ///   :trait      : Return the mutual information with another provided trait.

  MABE::trait_summary_t MABE::BuildTraitSummary(
    const std::string & trait_name,
    std::string trait_filter
  ) {
    // The trait input has two components:
    // (1) the trait NAME and
    // (2) (optionally) how to calculate the trait SUMMARY, such as min, max, ave, etc.

    // Everything before the first colon is the trait name.
    size_t trait_id = org_data_map.GetID(trait_name);
    emp::TypeID trait_type = org_data_map.GetType(trait_id);
    const bool is_numeric = trait_type.IsArithmetic();

    auto get_double_fun = [trait_id, trait_type](const Organism & org) {
      return org.GetTraitAsDouble(trait_id, trait_type);
    };
    auto get_string_fun = [trait_id, trait_type](const Organism & org) {
      return emp::to_literal( org.GetTraitAsString(trait_id, trait_type) );
    };

    // Return the number of times a specific value was found.
    if (trait_filter[0] == '=') {
      // @CAO: DO THIS!
      trait_filter.erase(0,1); // Erase the '=' and we are left with the string to match.
    }

    // Otherwise pass along to the BuildCollectFun with the correct type...
    auto result = is_numeric
                ? emp::BuildCollectFun<double,      Collection>(trait_filter, get_double_fun)
                : emp::BuildCollectFun<std::string, Collection>(trait_filter, get_string_fun);

    // If we made it past the 'if' statements, we don't know this aggregation type.
    if (!result) {
      error_man.AddError("Unknown trait filter '", trait_filter, "' for trait '", trait_name, "'.");
      return [](const Collection &){ return std::string("Error! Unknown trait function"); };
    }

    return result;
  }

  // Handler for printing trait data
  void MABE::OutputTraitData(std::ostream & os,
                             Collection target_collect,
                             std::string format,
                             bool print_headers)
  {
    emp::vector<trait_summary_t> trait_functions;  ///< Summary functions to call each update.
    emp::remove_whitespace(format);

    // If we need headers, set them up!
    if (print_headers) {
      // Identify the contents of each column.
      emp::vector<std::string> cols = emp::slice(format, ',');

      // Print the headers into the file.
      os << "#update";
      for (size_t i = 0; i < cols.size(); i++) {
        os << ", " << cols[i];
      }
      os << '\n';
    }

    // Pre-process the format to deal with config variables that need translating.
    format = Preprocess(format);

    // Check the cache for the functions to run; if they don't exist yet, set them up!
    auto fun_it = file_fun_cache.find(format);
    if (fun_it == file_fun_cache.end()) {
      // Identify the contents of each column.
      emp::vector<std::string> cols = emp::slice(format, ',');

      // Setup a function to collect data associated with each column.
      trait_functions.resize(cols.size());
      for (size_t i = 0; i < cols.size(); i++) {
        std::string trait_filter = cols[i];
        std::string trait_name = emp::string_pop(trait_filter,':');
        trait_functions[i] = BuildTraitSummary(trait_name, trait_filter);
      }

      // Insert the new entry into the cache and update the iterator.
      fun_it = file_fun_cache.insert({format, trait_functions}).first;
    }
    else trait_functions = fun_it->second;

    // And, finally, print the data!
    os << GetUpdate();
    for (auto & fun : trait_functions) {
      os << ", " << fun(target_collect);
    }
    os << std::endl;
  }

  void MABE::SetupConfig() {
    emp_assert(cur_scope);
    emp_assert(cur_scope.Raw() == &(config.GetRootScope()),
                cur_scope->GetName(),
                config.GetRootScope().GetName());  // Scope should start at root level.

    // Setup main MABE variables.
    cur_scope->LinkFuns<int>("random_seed",
                             [this](){ return random.GetSeed(); },
                             [this](int seed){ random.ResetSeed(seed); },
                             "Seed for random number generator; use 0 to base on time.");
  }


  bool MABE::OK() {
    bool result = true;

    // Make sure the populations are all OK.
    for (size_t pop_id = 0; pop_id < pops.size(); pop_id++) {
      result &= pops[pop_id]->OK();
    }

    // @CAO: Should check to make sure modules are okay too.

    return result;
  }


}

#endif
