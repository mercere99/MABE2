/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  MABE.h
 *  @brief Master controller object for a MABE run.
 *
 *  This is the master MABE controller object that hooks together all of the modules and provides
 *  the interface for them to interact.
 *
 *  MABE allows modules to interact via a set of SIGNALS that they can listen for by overriding
 *  specific base class functions.  See Module.h for a full list of available signals.
 *
 */

#ifndef MABE_MABE_H
#define MABE_MABE_H

#include <string>
#include <sstream>

#include "base/array.h"
#include "base/Ptr.h"
#include "base/vector.h"
#include "config/command_line.h"
#include "control/Signal.h"
#include "data/DataMap.h"
#include "tools/Random.h"
#include "tools/vector_utils.h"

#include "../config/Config.h"

#include "Collection.h"
#include "ModuleBase.h"
#include "Population.h"

namespace mabe {

  /// MABEBase sets up all population-manipulation functionality to ensure consistant handling
  /// (nowhere else can alter a Population object); all manipulation calls ultimately come through
  /// the limited functions defined here.  MABEBase is a friend class of Population, with all
  /// operations to manipulate organisms is a population listed as private.

  class MABEBase {
  protected:
    using mod_ptr_t = emp::Ptr<ModuleBase>;

    /// A SigListener tracks which Modules respond to a specific signal.  They maintain pointers
    /// to modules and call them when requested.  The base class manages common functionality.
    struct SigListenerBase : public emp::vector<mod_ptr_t> {
      std::string name;             ///< Name of this signal type.
      ModuleBase::SignalID id;      ///< ID of this signal
      mod_ptr_t cur_mod; ///< Which module is currently running?

      SigListenerBase(const std::string & _name="",
                    ModuleBase::SignalID _id=ModuleBase::SIG_UNKNOWN)
        : name(_name), id(_id) {;}
      SigListenerBase(const SigListenerBase &) = default;
      SigListenerBase(SigListenerBase &&) = default;
      SigListenerBase & operator=(const SigListenerBase &) = default;
      SigListenerBase & operator=(SigListenerBase &&) = default;
    };

    /// Each set of modules to be called when a specific signal is triggered should be identified
    /// in a SigListener object that has full type information.
    template <typename RETURN, typename... ARGS>
    struct SigListener : public SigListenerBase {

      /// Define the proper signal call type.
      typedef RETURN (ModuleBase::*ModMemFun)(ARGS...);

      /// Store the member-function call that this SigListener should handle.
      ModMemFun fun;

      /// A SigListener constructor takes both the member function that its supposed to call
      /// and a master list of module vectors that it should put itself it.
      SigListener(const std::string & _name,
                ModuleBase::SignalID _id,
                ModMemFun _fun,
                emp::array< emp::Ptr<SigListenerBase>, (size_t) ModuleBase::NUM_SIGNALS> & _ptrs)
        : SigListenerBase(_name, _id), fun(_fun)
      {
        _ptrs[id] = this;
      }

      template <typename... ARGS2>
      void Trigger(ARGS2 &&... args) {
        for (mod_ptr_t mod_ptr : *this) {
          cur_mod = mod_ptr;
          emp_assert(!mod_ptr.IsNull());
          (mod_ptr.Raw()->*fun)( std::forward<ARGS2>(args)... );
        }
        cur_mod = nullptr;
      }

      template <typename... ARGS2>
      OrgPosition FindPosition(ARGS2 &&... args) {
        OrgPosition result;
        for (mod_ptr_t mod_ptr : *this) {
          result = (mod_ptr.Raw()->*fun)(std::forward<ARGS2>(args)...);
          if (result.IsValid()) break;
        }
        return result;
      }
    };

    /// Maintain a master array of pointers to all SigListeners.
    emp::array< emp::Ptr<SigListenerBase>, (size_t) ModuleBase::NUM_SIGNALS > sig_ptrs;

    /// Maintain a collection of all modules used in this run.
    emp::vector<mod_ptr_t> modules;  

    // --- Track which modules need to have each signal type called on them. ---
    // BeforeUpdate(size_t update_ending)
    SigListener<void,size_t> before_update_sig;
    // OnUpdate(size_t new_update)
    SigListener<void,size_t> on_update_sig;
    // BeforeRepro(OrgPosition parent_pos) 
    SigListener<void,OrgPosition> before_repro_sig;
    // OnOffspringReady(Organism & offspring, OrgPosition parent_pos, Population & target_pop)
    SigListener<void,Organism &, OrgPosition, Population &> on_offspring_ready_sig;
    // OnInjectReady(Organism & inject_org, Population & target_pop)
    SigListener<void,Organism &, Population &> on_inject_ready_sig;
    // BeforePlacement(Organism & org, OrgPosition target_pos, OrgPosition parent_pos)
    SigListener<void,Organism &, OrgPosition, OrgPosition> before_placement_sig;
    // OnPlacement(OrgPosition placement_pos)
    SigListener<void,OrgPosition> on_placement_sig;
    // BeforeMutate(Organism & org)
    SigListener<void,Organism &> before_mutate_sig; // TO IMPLEMENT
    // OnMutate(Organism & org)
    SigListener<void,Organism &> on_mutate_sig; // TO IMPLEMENT
    // BeforeDeath(OrgPosition remove_pos)
    SigListener<void,OrgPosition> before_death_sig;
    // BeforeSwap(OrgPosition pos1, OrgPosition pos2)
    SigListener<void,OrgPosition,OrgPosition> before_swap_sig;
    // OnSwap(OrgPosition pos1, OrgPosition pos2)
    SigListener<void,OrgPosition,OrgPosition> on_swap_sig;
    // BeforePopResize(Population & pop, size_t new_size)
    SigListener<void,Population &, size_t> before_pop_resize_sig;
    // OnPopResize(Population & pop, size_t old_size)
    SigListener<void,Population &, size_t> on_pop_resize_sig;
    // OnError(const std::string & msg)
    SigListener<void,const std::string &> on_error_sig;
    // OnWarning(const std::string & msg)
    SigListener<void,const std::string &> on_warning_sig;
    // BeforeExit()
    SigListener<void> before_exit_sig;
    // OnHelp()
    SigListener<void> on_help_sig;

    // OrgPosition DoPlaceBirth(Organism & offspring, OrgPosition parent_position, Population & target_pop);
    SigListener<OrgPosition, Organism &, OrgPosition, Population &> do_place_birth_sig;
    // OrgPosition DoPlaceInject(Organism & new_organism)
    SigListener<OrgPosition, Organism &, Population &> do_place_inject_sig;
    // OrgPosition DoFindNeighbor(OrgPosition target_organism) {
    SigListener<OrgPosition, OrgPosition> do_find_neighbor_sig;

    /// If a module fails to use a signal, we never check it again UNLESS we are explicitly
    /// told to rescan the signals (perhaps because new functionality was enabled.)
    bool rescan_signals = true;

    // Protected constructor so that base class cannot be instantiated except from derived class.
    MABEBase()
    : before_update_sig("before_update", ModuleBase::SIG_BeforeUpdate, &ModuleBase::BeforeUpdate, sig_ptrs)
    , on_update_sig("on_update", ModuleBase::SIG_OnUpdate, &ModuleBase::OnUpdate, sig_ptrs)
    , before_repro_sig("before_repro", ModuleBase::SIG_BeforeRepro, &ModuleBase::BeforeRepro, sig_ptrs)
    , on_offspring_ready_sig("on_offspring_ready", ModuleBase::SIG_OnOffspringReady, &ModuleBase::OnOffspringReady, sig_ptrs)
    , on_inject_ready_sig("on_inject_ready", ModuleBase::SIG_OnInjectReady, &ModuleBase::OnInjectReady, sig_ptrs)
    , before_placement_sig("before_placement", ModuleBase::SIG_BeforePlacement, &ModuleBase::BeforePlacement, sig_ptrs)
    , on_placement_sig("on_placement", ModuleBase::SIG_OnPlacement, &ModuleBase::OnPlacement, sig_ptrs)
    , before_mutate_sig("before_mutate", ModuleBase::SIG_BeforeMutate, &ModuleBase::BeforeMutate, sig_ptrs)
    , on_mutate_sig("on_mutate", ModuleBase::SIG_OnMutate, &ModuleBase::OnMutate, sig_ptrs)
    , before_death_sig("before_death", ModuleBase::SIG_BeforeDeath, &ModuleBase::BeforeDeath, sig_ptrs)
    , before_swap_sig("before_swap", ModuleBase::SIG_BeforeSwap, &ModuleBase::BeforeSwap, sig_ptrs)
    , on_swap_sig("on_swap", ModuleBase::SIG_OnSwap, &ModuleBase::OnSwap, sig_ptrs)
    , before_pop_resize_sig("before_pop_resize", ModuleBase::SIG_BeforePopResize, &ModuleBase::BeforePopResize, sig_ptrs)
    , on_pop_resize_sig("on_pop_resize", ModuleBase::SIG_OnPopResize, &ModuleBase::OnPopResize, sig_ptrs)
    , on_error_sig("on_error", ModuleBase::SIG_OnError, &ModuleBase::OnError, sig_ptrs)
    , on_warning_sig("on_warning", ModuleBase::SIG_OnWarning, &ModuleBase::OnWarning, sig_ptrs)
    , before_exit_sig("before_exit", ModuleBase::SIG_BeforeExit, &ModuleBase::BeforeExit, sig_ptrs)
    , on_help_sig("on_help", ModuleBase::SIG_OnHelp, &ModuleBase::OnHelp, sig_ptrs)
    , do_place_birth_sig("do_place_birth", ModuleBase::SIG_DoPlaceBirth, &ModuleBase::DoPlaceBirth, sig_ptrs)
    , do_place_inject_sig("do_place_inject", ModuleBase::SIG_DoPlaceInject, &ModuleBase::DoPlaceInject, sig_ptrs)
    , do_find_neighbor_sig("do_find_neighbor", ModuleBase::SIG_DoFindNeighbor, &ModuleBase::DoFindNeighbor, sig_ptrs)
    { ;  }

  public:

    /// Setup signals to be rescanned; call this if any signal is updated in a module.
    void RescanSignals() { rescan_signals = true; }

    /// All insertions of organisms into a population should come through AddOrgAt
    /// @param[in] org_ptr points to the organism being added (which will now be owned by the population).
    /// @param[in] pos is the position to perform the insertion.
    /// @param[in] ppos is the parent position (required if it exists; not used with inject).
    void AddOrgAt(emp::Ptr<Organism> org_ptr, OrgPosition pos, OrgPosition ppos=OrgPosition()) {
      emp_assert(org_ptr);  // Must have a non-null organism to insert.
      before_placement_sig.Trigger(*org_ptr, pos, ppos);
      ClearOrgAt(pos);      // Clear out any organism already in this position.
      pos.PopPtr()->SetOrg(pos.Pos(), org_ptr);  // Put the new organism in place.
      on_placement_sig.Trigger(pos);
    }

    /// All permanent deletion of organisms from a population should come through here.
    /// If the relavant position is already empty, nothing happens.
    /// @param[in] pos is the position to perform the deletion.
    void ClearOrgAt(OrgPosition pos) {
      emp_assert(pos.IsValid());
      if (pos.IsEmpty()) return; // Nothing to remove!

      before_death_sig.Trigger(pos);
      pos.PopPtr()->ExtractOrg(pos.Pos()).Delete();
    }

    /// All movement of organisms from one population position to another should come through here.
    void SwapOrgs(OrgPosition pos1, OrgPosition pos2) {
      emp_assert(pos1.IsValid());
      emp_assert(pos2.IsValid());
      before_swap_sig.Trigger(pos1, pos2);
      emp::Ptr<Organism> org1 = pos1.PopPtr()->ExtractOrg(pos1.Pos());
      emp::Ptr<Organism> org2 = pos2.PopPtr()->ExtractOrg(pos2.Pos());
      if (!org1->IsEmpty()) pos2.PopPtr()->SetOrg(pos2.Pos(), org1);
      if (!org2->IsEmpty()) pos1.PopPtr()->SetOrg(pos1.Pos(), org2);
      on_swap_sig.Trigger(pos1, pos2);
    }

    /// Change the size of a population.  If shrinking, clear orgs at removed positions;
    /// if growing, new positions will have empty organisms.
    void ResizePop(Population & pop, size_t new_size) {
      // Clean up any organisms that may be getting deleted.
      const size_t old_size = pop.GetSize();                // Track the starting size.
      if (old_size == new_size) return;                     // If size isn't changing, we're done!

      before_pop_resize_sig.Trigger(pop, new_size);         // Signal that resize about to happen.

      for (size_t pos = new_size; pos < old_size; pos++) {  // Clear all orgs out of range.
        ClearOrgAt( OrgPosition(pop, pos) );
      }

      pop.Resize(new_size);                                 // Do the actual resize.

      on_pop_resize_sig.Trigger(pop, old_size);             // Signal that resize has happened.
    }

    /// Add a single, empty position onto the end of a population.
    PopIterator PushEmpty(Population & pop) {
      before_pop_resize_sig.Trigger(pop, pop.GetSize()+1);
      PopIterator it = pop.PushEmpty();
      on_pop_resize_sig.Trigger(pop, pop.GetSize()-1);
      return it;
    }
  };


  /////////////////////////////////////////////////////////////////////////////
  ///
  /// The main MABE controller class
  ///

  class MABE : public MABEBase {
  private:
    const std::string VERSION = "0.0.1";

    /// Populations used; generated based on the needs of modules.
    emp::vector<Population> pops;

    /// Organism pointer to use for all empty cells.
    emp::Ptr<Organism> empty_org = nullptr;

    /// Information about organism traits.  TraitInfo specifies which modules are allowed to
    /// (or expected to) access each trait, as well as how that trait should be initialized,
    /// archived, and summarized.
    std::unordered_map<std::string, emp::Ptr<TraitInfo>> trait_map;

    /// Trait information to be stored on each organism.  This is the prototype map, which
    /// tracks the name, type, and current value of all traits that modules associate with
    /// organisms.
    emp::DataMap org_data_map;

    emp::Random random;                ///< Master random number generator
    int random_seed = 0;               ///< Random number seed used for this run.
    size_t cur_pop_id = (size_t) -1;   ///< Which population is currently active?
    size_t update = 0;                 ///< How many times has Update() been called?


    // --- Variables to handle configuration and initialization ---

    bool verbose = false;              ///< Should we output extra information during setup?
    emp::vector<std::string> errors;   ///< Log any errors that have occured.
    bool show_help = false;            ///< Should we show "help" before exiting?
    bool exit_now = false;             ///< Do we need to immediately clean up and exit the run?

    using mod_ptr_t = emp::Ptr<ModuleBase>;

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
    std::string gen_filename;                  ///< Name of output file to generate.
    Config config;                             ///< Configutation information for this run.
    emp::Ptr<ConfigScope> cur_scope;           ///< Which config scope are we currently using?

    // Helpers to ensure proper configuration ordering.
    bool allow_trait_linking = false;  ///< Modules should link traits only AFTER config is run.


    // ----------- Helper Functions -----------

    /// Call when ready to end a run.
    void Exit() {
      // Let all modules know that exit is about to occur.
      before_exit_sig.Trigger();

      // @CAO: Other local cleanup in case destructor is not run due to early termination?

      // Exit as soon as possible.
      exit_now = true;
    }

    /// Print information on how to run the software.
    void ShowHelp() {
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
      Exit();
    }

    /// List all of the available modules included in the current compilation.
    void ShowModules() {
      std::cout << "MABE v" << VERSION << "\n"
                << "Active modules:\n";
      for (auto mod_ptr : modules) {
        std::cout << "  " << mod_ptr->GetName() << " : " << mod_ptr->GetDesc() << "\n";
      }          
      std::cout << "All available modules:\n";
      for (auto & info : GetModuleInfo()) {
        std::cout << "  " << info.name << " : " << info.desc << "\n";
      }          
      Exit();
    }

    /// Process all of the arguments that were passed in on the command line.
    void ProcessArgs();

    // -- Helper functions to be called inside of Setup() --

    /// Run SetupModule() method on each module we've loaded.
    void Setup_Modules();

    /// Load organism traits that modules need to read or write and test for conflicts.
    void Setup_Traits();

    /// Link signals to the modules that implment responses to those signals.
    void UpdateSignals();


    // -- Helper functions for debugging and extra output --

    /// Output args if (and only if) we are in verbose mode.
    template <typename... Ts>
    void verbose_out(Ts &&... args) {
      if (verbose) {
        std::cout << emp::to_string(std::forward<Ts>(args)...) << std::endl;
      }
    }

  public:
    MABE(int argc, char* argv[]);  ///< MABE command-line constructor.
    MABE(const MABE &) = delete;
    MABE(MABE &&) = delete;
    ~MABE() {
      if (empty_org) empty_org.Delete();                           // Delete empty_org ptr.
      for (auto x : modules) x.Delete();                           // Delete all modules.
      for (auto [name,trait_ptr] : trait_map) trait_ptr.Delete();  // Delete all trait info.
    }

    // --- Basic accessors ---
    emp::Random & GetRandom() { return random; }
    size_t GetUpdate() const noexcept { return update; }

    // --- Tools to setup runs ---
    bool Setup();

    /// Setup an organism as a placeholder for all "empty" positions in the population.
    template <typename EMPTY_MANAGER_T>
    void SetupEmpty() {
      if (empty_org) empty_org.Delete();  // If we already have an empty organism, replace it.
      auto & empty_manager =
        AddModule<EMPTY_MANAGER_T>("EmptyOrg", "Manager for all 'empty' organisms in any population.");
      empty_manager.SetBuiltIn();         // Don't write the empty manager to config.

      empty_org = empty_manager.MakeOrganism();
    }

    /// Update MABE a single time step.
    void Update();

    /// Update MABE a specified number of time steps.
    void Update(size_t num_updates) {
      config.TriggerEvents("start");
      for (size_t ud = 0; ud < num_updates && !exit_now; ud++) {
        Update();
      }
    }

    // -- Error Handling --
    template <typename... Ts>
    void AddError(Ts &&... args) {
      // If we are in debug mode, trigger an error immediately.
      emp_error(args...);

      // Otherwise deal with it using the error modules.
      errors.push_back( emp::to_string( std::forward<Ts>(args)... ));
      on_error_sig.Trigger(errors.back());
    }
    void AddErrors(const emp::vector<std::string> & in_errors) {
      errors.insert(errors.end(), in_errors.begin(), in_errors.end());
    }

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
      return emp::FindEval(pops, [pop_name](const auto & p){ return p.GetName() == pop_name; });
    }
    const Population & GetPopulation(size_t id) const { return pops[id]; }
    Population & GetPopulation(size_t id) { return pops[id]; }

    /// New populaitons must be given a name and an optional size.
    Population & AddPopulation(const std::string & name, size_t pop_size=0) {
      cur_pop_id = (int) pops.size();                           // New population will be "current"
      pops.emplace_back(name, cur_pop_id, pop_size, empty_org); // Create the new population.
      return pops[cur_pop_id];                                  // Return the new population.
    }

    /// If GetPopulation() is called without an ID, return the current population or create one.
    Population & GetPopulation() {
      if (pops.size() == 0) {                   // If we don't have a population, add one!
        emp_assert(cur_pop_id == (size_t) -1);  // Current population should now be default;
        AddPopulation("main");                  // Default population is named main.
      }
      return pops[cur_pop_id];
    }

    /// Move an organism from one position to another; kill anything that previously occupied
    /// the target position.
    void MoveOrg(OrgPosition from_pos, OrgPosition to_pos) {
      ClearOrgAt(to_pos);
      SwapOrgs(from_pos, to_pos);
    }

    /// Inject a copy of the provided organism and return the position it was placed in;
    /// if more than one is added, return the position of the final injection.
    OrgPosition Inject(const Organism & org, Population & pop, size_t copy_count=1) {
      emp_assert(org.GetDataMap().SameLayout(org_data_map));
      OrgPosition pos;
      for (size_t i = 0; i < copy_count; i++) {
        emp::Ptr<Organism> inject_org = org.Clone();
        on_inject_ready_sig.Trigger(*inject_org, pop);
        pos = FindInjectPosition(*inject_org, pop);
        if (pos.IsValid()) {
          AddOrgAt( inject_org, pos);
        } else {
          inject_org.Delete();          
          AddError("Invalid position (pop=", pos.PopPtr(), "; pos=", pos.Pos(),
                   "); failed to inject organism ", i, "!");
        }
      }
      return pos;
    }

    /// Add an organsim of a specified type to the world (provide the type name and the
    /// MABE controller will create an instance of it.)
    OrgPosition Inject(const std::string & type_name, Population & pop, size_t copy_count=1) {
      auto & org_manager = GetModule(type_name);            // Look up type of organism.
      auto org_ptr = org_manager.MakeOrganism(random);      // Build an org of this type.
      OrgPosition pos = Inject(*org_ptr, pop, copy_count);  // Inject a copy of the organism.
      org_ptr.Delete();                                     // Delete generated organism.
      return pos;                                           // Return last position injected.
    }

    /// Add an organism of a specified type and population (provide names of both and they
    /// will be properly setup.)
    OrgPosition Inject(const std::string & type_name, 
                       const std::string & pop_name,
                       size_t copy_count=1) {      
      int pop_id = GetPopID(pop_name);
      if (pop_id == -1) {
        AddError("Invalid population name used in inject '", pop_name, "'.");        
      }
      Population & pop = GetPopulation(pop_id);
      OrgPosition pos = Inject(type_name, pop, copy_count);  // Inject a copy of the organism.
      return pos;                                           // Return last position injected.
    }

    /// Inject a copy of the provided organism at a specified position.
    void InjectAt(const Organism & org, OrgPosition pos) {
      emp_assert(pos.IsValid());
      emp::Ptr<Organism> inject_org = org.Clone();
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
                        bool do_mutations=true) {
      emp_assert(org.IsEmpty() == false);  // Empty cells cannot reproduce.
      before_repro_sig.Trigger(ppos);
      OrgPosition pos;                                      // Position of each offspring placed.
      emp::Ptr<Organism> new_org;
      for (size_t i = 0; i < birth_count; i++) {            // Loop through offspring, adding each
        new_org = do_mutations ? org.MakeOffspring(random) : org.Clone();

        // Alert modules that offspring is ready, then find its birth position.
        on_offspring_ready_sig.Trigger(*new_org, ppos, target_pop);
        pos = FindBirthPosition(*new_org, ppos, target_pop);

        // If this placement is valid, do so.  Otherwise delete the organism.
        if (pos.IsValid()) AddOrgAt(new_org, pos, ppos);
        else new_org.Delete();
      }
      return pos;
    }

    OrgPosition DoBirth(const Organism & org,
                        OrgPosition ppos,
                        OrgPosition target_pos,
                        bool do_mutations=true) {
      emp_assert(org.IsEmpty() == false);  // Empty cells cannot reproduce.
      emp_assert(target_pos.IsValid());    // Target positions must already be valid.

      before_repro_sig.Trigger(ppos);
      emp::Ptr<Organism> new_org = do_mutations ? org.MakeOffspring(random) : org.Clone();
      on_offspring_ready_sig.Trigger(*new_org, ppos, target_pos.Pop());

      AddOrgAt(new_org, target_pos, ppos);

      return target_pos;
    }


    /// A shortcut to DoBirth where only the parent position needs to be supplied.
    OrgPosition Replicate(OrgPosition ppos, Population & target_pop,
                          size_t birth_count=1, bool do_mutations=true) {
      return DoBirth(*ppos, ppos, target_pop, birth_count, do_mutations);
    }

    /// Resize a population while clearing all of the organisms in it.
    void EmptyPop(Population & pop, size_t new_size) {
      // Clean up any organisms in the population.
      for (PopIterator pos = pop.begin(); pos != pop.end(); ++pos) {
        ClearOrgAt(pos);
      }

      MABEBase::ResizePop(pop, new_size);
    }

    /// Return a ramdom position from a desginated population.
    OrgPosition GetRandomPos(Population & pop) {
      emp_assert(pop.GetSize() > 0);
      return pop.IteratorAt( random.GetUInt(pop.GetSize()) );
    }

    /// Return a ramdom position from the population with the specified id.
    OrgPosition GetRandomPos(size_t pop_id) { return GetRandomPos(GetPopulation(pop_id)); }

    /// Return a ramdom position from a desginated population with a living organism in it.
    OrgPosition GetRandomOrgPos(Population & pop) {
      emp_assert(pop.GetNumOrgs() > 0, "GetRandomOrgPos cannot be called if there are no orgs.");
      // @CAO: Something better to do in a sparse population?
      OrgPosition pos = GetRandomPos(pop);
      while (pos.IsEmpty()) pos = GetRandomPos(pop);
      return pos;
    }

    /// Return a ramdom position of a living organism from the population with the specified id.
    OrgPosition GetRandomOrgPos(size_t pop_id) { return GetRandomOrgPos(GetPopulation(pop_id)); }


    // --- Collection Management ---

    std::string ToString(const mabe::Collection & collect) const {
      return collect.ToString();
    }

    Collection FromString(const std::string & load_str) {
      Collection out;
      auto slices = emp::view_slices(load_str, ',');
      for (auto name : slices) {
        int pop_id = GetPopID(name);
        if (pop_id == -1) AddError("Unknown population: ", name);
        else out.Insert(GetPopulation(pop_id));
      }
      return out;
    }

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

    /// Add a new organism trait.
    template <typename T>
    TypedTraitInfo<T> & AddTrait(emp::Ptr<ModuleBase> mod_ptr,
                                 TraitInfo::Access access,
                                 const std::string & trait_name,
                                 const std::string & desc,
                                 const T & default_val)
    {
      emp::Ptr<TypedTraitInfo<T>> cur_trait = nullptr;
      const std::string & mod_name = mod_ptr->GetName();

      if (allow_trait_linking == false) {
        AddError("Module '", mod_name, "' adding trait '", trait_name,
                 "' before config files have loaded; should be done in SetupModule().");
      }

      // Traits cannot be added without access information.
      if (access == TraitInfo::UNKNOWN) {
        AddError("Module ", mod_name, " trying to add trait named '", trait_name,
                 "' with UNKNOWN access type.");
      }

      // If the trait does not already exist, build it as a new trait.
      if (emp::Has(trait_map, trait_name) == false) {
        cur_trait = emp::NewPtr<TypedTraitInfo<T>>(trait_name);
        cur_trait->SetDesc(desc);
        trait_map[trait_name] = cur_trait;
        org_data_map.AddVar(trait_name, default_val, desc);
      }
      
      // Otherwise make sure that it is consistent with previous modules.
      else {
        cur_trait = trait_map[trait_name].DynamicCast<TypedTraitInfo<T>>();

        // Make sure that the SAME module isn't defining a trait twice.
        if (cur_trait->HasAccess(mod_ptr)) {
          AddError("Module ", mod_name, " is creating multiple traits named '",
                   trait_name, "'.");
        }

        // Make sure the type is consistent across modules.
        if (cur_trait->GetType() != emp::GetTypeID<T>()) {
          AddError("Module ", mod_name, " is trying to use trait '",
                   trait_name, "' of type ", emp::GetTypeID<T>(),
                   "; Previously defined in module(s) ",
                   emp::to_english_list(cur_trait->GetModuleNames()),
                   " as type ", cur_trait->GetType());
        }
      }

      // Add this modules access to the trait.
      cur_trait->AddAccess(mod_name, mod_ptr, access);

      return *cur_trait;
    }

    using trait_fun_t = std::function<std::string(const Collection &)>;

    /// Determine if a trait is of a numeric type that MABE supports.
    bool IsNumericTypeID(emp::TypeID type_id) {
      if (type_id == emp::GetTypeID<bool>()) return true;
      if (type_id == emp::GetTypeID<double>()) return true;
      if (type_id == emp::GetTypeID<int>()) return true;
      if (type_id == emp::GetTypeID<size_t>()) return true;
      return false;
    }

    // /// Generate a function that will find and return the minimum value of a trait as a string.
    // trait_fun_t GetTraitFunction_Min(const std::string & trait_name) {
    //   size_t trait_id = org_data_map.GetID(trait_name);      
    //   emp::TypeID trait_type = org_data_map.GetType(trait_id);

    //   if (trait_type == emp::GetTypeID<int>()) {
    //     return [trait_id](const Collection & collect){
    //       return std::string("Error: non-numeric trait.");
    //     };
    //   }

    //   return [trait_id](const Collection &){ return std::string("Error: non-numeric trait."); };
    // }

    // /// Generate a function -- after knowing a trait name and type -- that will take a collection
    // /// and return the current value of this trait as a string.
    // template <typename T>
    // trait_fun_t GetTraitFunction(size_t trait_id, const std::string & fun_name) {
    //   // The remainder indicates how to aggregate the trait.
    //   if (fun_name == "min") {
    //     return GetTraitFunction_Min(name);
    //   }

    //   // If we made it past the 'if' statements, we don't know this aggregation type.
    //   AddError("Unknown trait aggregation mode '", trait_input, "' for trait '", name, "'.");

    //   return [](const Collection &){ return std::string("Error! Unknown trait function"); };
    // }

    // /// Parse a descriptor to Generate a function that will take a collection and return the
    // /// current value of this trait as a string.
    // trait_fun_t ParseTraitFunction(std::string trait_input) {
    //   // The trait input has two components: the trait name and the trait type (min, max, ave)

    //   // Everything before the first colon is the trait name.
    //   std::string trait_name = emp::string_pop(trait_input,':');
    //   size_t trait_id = org_data_map.GetId(trait_name);      

    //   if (type_id == emp::GetTypeID<bool>()) {
    //     return GetTraitFunction<bool>(trait_id, trait_input);
    //   }
    //   if (type_id == emp::GetTypeID<double>()) {
    //     return GetTraitFunction<double>(trait_id, trait_input);
    //   }
    //   if (type_id == emp::GetTypeID<int>()) {
    //     return GetTraitFunction<int>(trait_id, trait_input);
    //   }
    //   if (type_id == emp::GetTypeID<size_t>()) {
    //     return GetTraitFunction<size_t>(trait_id, trait_input);
    //   }


    //   // If we made it past the 'if' statements, we don't know this aggregation type.
    //   AddError("Unknown trait aggregation mode '", trait_input, "' for trait '", name, "'.");

    //   return [](const Collection &){ return std::string("Error! Unknown trait function"); };
    // }

    // --- Manage configuration scope ---

    /// Access to the current configuration scope.
    ConfigScope & GetCurScope() { return *cur_scope; }

    /// Add a new scope under the current one.
    ConfigScope & AddScope(const std::string & name, const std::string & desc) {
      cur_scope = &(cur_scope->AddScope(name, desc));
      return *cur_scope;
    }

    /// Move up one level of scope.
    ConfigScope & LeaveScope() {
      cur_scope = cur_scope->GetScope();
      return *cur_scope;
    }

    /// Return to the root scope.
    ConfigScope & ResetScope() {
      cur_scope = &(config.GetRootScope());
      return *cur_scope;
    }

    /// Setup the configuration options for MABE, including for each module.
    void SetupConfig();

    /// Do some basic sanity checks for debugging; return whether all details of the current
    /// MABE setup are "okay".
    bool OK();


    // Checks for which modules are currently being triggered.

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
    bool OnHelp_IsTriggered(mod_ptr_t mod) { return on_help_sig.cur_mod == mod; };
    bool DoPlaceBirth_IsTriggered(mod_ptr_t mod) { return do_place_birth_sig.cur_mod == mod; };
    bool DoPlaceInject_IsTriggered(mod_ptr_t mod) { return do_place_inject_sig.cur_mod == mod; };
    bool DoFindNeighbor_IsTriggered(mod_ptr_t mod) { return do_find_neighbor_sig.cur_mod == mod; };
  };


  // ========================== OUT-OF-CLASS DEFINITIONS! ==========================

  MABE::MABE(int argc, char* argv[])
    : args(emp::cl::args_to_strings(argc, argv))
    , cur_scope(&(config.GetRootScope()))
  {
    // Setup "Population" as a type in the config file.
    std::function<ConfigType &(const std::string &)> pop_init_fun =
      [this](const std::string & name) -> ConfigType & {
        return AddPopulation(name);
      };
    config.AddType("Population", "Collection of organisms", pop_init_fun);

    // Setup all known modules as types in the config file.
    for (auto & mod : GetModuleInfo()) {
      std::function<ConfigType &(const std::string &)> mod_init_fun =
        [this,&mod](const std::string & name) -> ConfigType & {
          return mod.init_fun(*this,name);
        };
      config.AddType(mod.name, mod.desc, mod_init_fun);
    }


    // Add other built-in functions to the config file.

    // 'exit' should terminate a run.
    std::function<int()> exit_fun = [this](){ Exit(); return 0; };
    config.AddFunction("exit", exit_fun, "Exit from this MABE run.");


    // 'inject' allows a user to add an organism to a population.
    std::function<int(const std::string &, const std::string &, size_t)> inject_fun =
      [this](const std::string & org_type_name, const std::string & pop_name, size_t count) {
        Inject(org_type_name, pop_name, count);
        return 0;
      };
    config.AddFunction("inject", inject_fun,
      "Inject organisms into a population (args: org_name, pop_name, org_count).");

    // 'print' is a simple debugging command to output the value of a variable.
    std::function<int(const emp::vector<emp::Ptr<ConfigEntry>> &)> print_fun =
      [](const emp::vector<emp::Ptr<ConfigEntry>> & args) {
        for (auto entry_ptr : args) std::cout << entry_ptr->AsString();
        return 0;
      };
    config.AddFunction("print", print_fun, "Print out the provided variable.");

    // Add in built-in event triggers; these are used to say when events should happen.
    config.AddEventType("start");   // Triggered at the beginning of a run.
    config.AddEventType("update");  // Tested every update.
  }

  bool MABE::Setup() {
    SetupConfig();                   // Load all of the parameters needed by modules, etc.
    ProcessArgs();                   // Deal with command-line inputs.

    // Sometime command-line arguments will require an immediate exit (such as after '--help')
    if (exit_now) return false;

    // If configuration filenames have been specified, load each of them in order.
    if (config_filenames.size()) {
      std::cout << "Loading file(s): " << emp::to_quoted_list(config_filenames) << std::endl;
      config.Load(config_filenames);   // Load files
    }

    // If we are writing a file, do so and then exit.
    if (gen_filename != "") {
      std::cout << "Generating file '" << gen_filename << "'." << std::endl;
      config.Write(gen_filename);
      Exit();
    }

    // If any of the inital flags triggered an 'exit_now', do so.
    if (exit_now) return false;

    // Allow traits to be linked.
    allow_trait_linking = true;

    Setup_Modules();        // Run SetupModule() on each module for linking traits or other setup.
    Setup_Traits();         // Make sure module traits do not clash.

    UpdateSignals();        // Setup the appropriate modules to be linked with each signal.

    // Collect errors in any module.
    for (emp::Ptr<ModuleBase> mod_ptr : modules) {
      if (mod_ptr->HasErrors()) { AddErrors(mod_ptr->GetErrors()); }
    }

    return true;
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

  void MABE::ProcessArgs() {
    arg_set.emplace_back("--filename", "-f", "[filename...] ", "Filenames of configuration settings",
      [this](const emp::vector<std::string> & in){ config_filenames = in; } );
    arg_set.emplace_back("--generate", "-g", "[filename]    ", "Generate a new output file",
      [this](const emp::vector<std::string> & in) {
        if (in.size() != 1) {
          std::cout << "'--generate' must be followed by a single filename.\n";
          Exit();
        } else {
          // MABE Config files should be generated FROM a *.gen file, typically creating a *.mabe
          // file.  If output file is *.gen assume an error. (for now; override should be allowed)
          if (in[0].size() > 4 && in[0].substr(in[0].size()-4) == ".gen") {
            AddError("Error: generated file ", in[0], " not allowed to be *.gen; typically should end in *.mabe.");
            Exit();
          }
          else gen_filename = in[0];
        }
      });
    arg_set.emplace_back("--help", "-h", "              ", "Help; print command-line options for MABE",
      [this](const emp::vector<std::string> &){ show_help = true; } );
    arg_set.emplace_back("--modules", "-m", "              ", "Module list",
      [this](const emp::vector<std::string> &){ ShowModules(); } );
    // arg_set.emplace_back("--set", "-s", "[param=value] ", "Set specified parameter",
    //   [this](const emp::vector<std::string> &){ emp_assert(false); } );
    arg_set.emplace_back("--verbose", "-+", "              ", "Output extra setup info",
      [this](const emp::vector<std::string> &){ verbose = true; } );
    arg_set.emplace_back("--version", "-v", "              ", "Version ID of MABE",
      [this](const emp::vector<std::string> &){
        std::cout << "MABE v" << VERSION << "\n";
        Exit();
      });

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
    verbose_out("Analyzing configuration of ", trait_map.size(), " traits.");

    // STEP 1: Make sure modules are accessing traits correctly and consistently.
    int error_count = 0;

    // Loop through all of the traits to ensure there are no conflicts.
    for (auto [trait_name, trait_ptr] : trait_map) {
      verbose_out("...scanning '", trait_name, "' with ", trait_ptr->GetModuleCount(), " modules:",
                  " private=", trait_ptr->GetPrivateCount(),
                  " owned=", trait_ptr->GetOwnedCount(),
                  " generated=", trait_ptr->GetGeneratedCount(),
                  " shared=", trait_ptr->GetSharedCount(),
                  " required=", trait_ptr->GetRequiredCount()
                 );

      // NO traits should be of UNKNOWN access.
      if (trait_ptr->GetUnknownCount()) {
        AddError("Unknown access mode for trait '", trait_name,
                 "' in module(s) ", emp::to_english_list(trait_ptr->GetUnknownNames()),
                 " (internal error!)");
        error_count++;
        continue;
      }

      // Only one module can be involved for PRIVATE access.
      else if (trait_ptr->GetPrivateCount() > 1) {
        std::stringstream error_msg;
        error_msg << "Multiple modules declaring trait '" << trait_name
                  << "' as private: " << emp::to_english_list(trait_ptr->GetPrivateNames())
                  << ".\n"
                  << "[Suggestion: if traits are supposed to be distinct, prepend names with a\n"
                  << " module-specific prefix.  Otherwise modules need to be edited to not have\n"
                  << " trait private.]";
        AddError(error_msg.str());
        error_count++;
        continue;
      }

      else if (trait_ptr->GetPrivateCount() && trait_ptr->GetModuleCount() > 1) {
        AddError("Trait '", trait_name, "' is private in module '", trait_ptr->GetPrivateNames()[0],
                 "'; should not be used by other modules.\n",
                 "[Suggestion: if traits are supposed to be distinct, prepend private name with a\n",
                 " module-specific prefix.  Otherwise module needs to be edited to not have\n",
                 " trait private.]");
        error_count++;
        continue;
      }

      // A trait that is OWNED or GENERATED cannot have other modules writing to it.
      else if (trait_ptr->GetOwnedCount() + trait_ptr->GetGeneratedCount() > 1) {
        auto mod_names = emp::Concat(trait_ptr->GetOwnedNames(), trait_ptr->GetGeneratedNames());
        std::stringstream error_msg;
        error_msg << "Multiple modules declaring ownership of trait '" << trait_name << "': "
                  << emp::to_english_list(mod_names) << ".\n"
                  << "[Suggestion: if traits are supposed to be distinct, prepend names with a\n"
                  << " module-specific prefix.  Otherwise modules should be edited to change trait\n"
                  << " to be SHARED (and all can modify) or have all but one shift to REQUIRED.]";
        AddError(error_msg.str());
        error_count++;
        continue;
      }

      else if ((trait_ptr->IsOwned() || trait_ptr->IsGenerated()) && trait_ptr->IsShared()) {
        AddError("Trait '", trait_name, "' is fully OWNED by module '", trait_ptr->GetOwnedNames()[0],
                 "'; it cannot be SHARED (written to) by other modules:",
                 emp::to_english_list(trait_ptr->GetSharedNames()),
                 "[Suggestion: if traits are supposed to be distinct, prepend private name with a\n",
                 " module-specific prefix.  Otherwise module needs to be edited to make trait\n",
                 " SHARED or have all but one shift to REQUIRED.]");
        error_count++;
        continue;
      }

      // A REQUIRED trait must have another module write to it (i.e. OWNED, GENERATED or SHARED).
      else if (trait_ptr->IsRequired() &&
              !trait_ptr->IsOwned() && !trait_ptr->IsShared() && !trait_ptr->IsGenerated()) {
        AddError("Trait '", trait_name, "' marked REQUIRED by module(s) ",
                 emp::to_english_list(trait_ptr->GetRequiredNames()),
                 "'; must be written to by other modules.\n",
                 "[Suggestion: set another module to write to this trait (where it is either\n",
                 " SHARED or OWNED).]");
        error_count++;
        continue;
      }

      // A GENERATED trait requires another module to read (REQUIRE) it.
      else if (trait_ptr->IsGenerated() && !trait_ptr->IsRequired()) {
        AddError("Trait '", trait_name, "' marked GENERATED by module(s) ",
                 emp::to_english_list(trait_ptr->GetGeneratedNames()),
                 "'; must be read by other modules.");
        error_count++;
        continue;
      }
    }

    // STEP 2: Lock in the DataMap and make sure that all of the modules (especially org managers)
    // are aware of the final set of traits.
    org_data_map.LockLayout();

    for (emp::Ptr<ModuleBase> mod_ptr : modules) {
      mod_ptr->SetupDataMap(org_data_map);
    }

    verbose_out("Trait error_count = ", error_count);
  }

  /// Link signals to the modules that implment responses to those signals.
  void MABE::UpdateSignals() {
    // Clear all module vectors.
    for (auto modv : sig_ptrs) modv->resize(0);

    // Loop through each module to update its signals.
    for (emp::Ptr<ModuleBase> mod_ptr : modules) {
      // For the current module, loop through all of the signals.
      for (size_t sig_id = 0; sig_id < sig_ptrs.size(); sig_id++) {
        if (mod_ptr->has_signal[sig_id]) sig_ptrs[sig_id]->push_back(mod_ptr);
      }
    }

    // Now that we have scanned the signals, we can turn off the rescan flag.
    rescan_signals = false;
  }

  void MABE::SetupConfig() {
    emp_assert(cur_scope);
    emp_assert(cur_scope.Raw() == &(config.GetRootScope()),
                cur_scope->GetName(),
                config.GetRootScope().GetName());  // Scope should start at root level.

    // Setup main MABE variables.
    cur_scope->LinkVar("random_seed",
                        random_seed,
                        "Seed for random number generator; use 0 to base on time.").SetMin(0);

    // Call the SetupConfig of module base classes (they will call the dervived version)
    for (auto m : modules) {
      if (m->IsBuiltIn()) continue;          // Built-in modules don't need to be configured.

      AddScope(m->GetName(), m->GetDesc());  // Add a config scope for each module we've created.
      m->SetupScope(*cur_scope);             // Notify the module about it's own scope.
      m->SetupConfig();                      // Allow to module to configure its scope.
      LeaveScope();                          // Exit the module's scope before move to next module.
    }
  }


  bool MABE::OK() {
    bool result = true;

    // Make sure the populations are all OK.
    for (size_t pop_id = 0; pop_id < pops.size(); pop_id++) {
      result &= pops[pop_id].OK();
    }

    // @CAO: Should check to make sure modules are okay too.

    return result;
  }

}

#endif
