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
 *  specific base class functions.
 *
 *  SIGNAL functions include:
 *    BeforeUpdate(size_t update_ending)
 *    OnUpdate(size_t new_update)
 *    BeforeRepro(OrgPosition parent_pos) 
 *    OnOffspringReady(Organism & offspring, OrgPosition parent_pos)
 *    OnInjectReady(Organism & inject_org)
 *    BeforePlacement(Organism & org, OrgPosition target_pos, OrgPosition parent_pos)
 *    OnPlacement(OrgPosition placement_pos)
 *    BeforeMutate(Organism & org)
 *    OnMutate(Organism & org)
 *    BeforeDeath(OrgPosition remove_pos)
 *    BeforeSwap(OrgPosition pos1, OrgPosition pos2)
 *    OnSwap(OrgPosition pos1, OrgPosition pos2)
 *    BeforePopResize(Population & pop, size_t new_size)
 *    OnPopResize(Population & pop, size_t old_size)
 *    OnError(const std::string & msg)
 *    OnWarning(const std::string & msg)
 *    BeforeExit()
 *    OnHelp()
 *
 *  There are also functions to find types of individuals including:
 *    OrgPosition DoPlaceBirth(Organism &, OrgPosition);
 *    OrgPosition DoPlaceInject(Organism &)
 *    OrgPosition DoFindNeighbor(OrgPosition) {
 *
 */

#ifndef MABE_MABE_H
#define MABE_MABE_H

#include <string>
#include <sstream>

#include "base/Ptr.h"
#include "base/vector.h"
#include "config/command_line.h"
#include "control/Signal.h"
#include "data/DataMap.h"
#include "tools/Random.h"
#include "tools/vector_utils.h"

#include "../config/Config.h"

#include "ModuleBase.h"
#include "OrganismManagerBase.h"
#include "Population.h"

namespace mabe {

  /// MABEBase sets up all population-manipulation functionality (nowhere else can alter a
  /// Population object).  As such, it guarantees that all manipulation calls ultimately
  /// come through the limited functions defined here.

  class MABEBase {
  protected:
    /// ModVectors track all of the Modules that are listening for a specific signal.
    /// They track a series of pointers to modules and handle them all being called.
    struct ModVectorBase : public emp::vector<emp::Ptr<ModuleBase>> {
      std::string name;          ///< Name of this signal type.
      ModuleBase::SignalID id;   ///< ID of this signal

      ModVectorBase(const std::string & _name="",
                    ModuleBase::SignalID _id=ModuleBase::SIG_UNKNOWN)
        : name(_name), id(_id) {;}
      ModVectorBase(const ModVectorBase &) = default;
      ModVectorBase(ModVectorBase &&) = default;
      ModVectorBase & operator=(const ModVectorBase &) = default;
      ModVectorBase & operator=(ModVectorBase &&) = default;
    };

    /// Maintain a master vector of all ModVector pointers to signals.
    static constexpr size_t num_signals = (size_t) ModuleBase::NUM_SIGNALS;
    emp::vector< emp::Ptr<ModVectorBase> > modv_ptrs;

    /// Each set of modules to be called when a specific signal is triggered should be identified
    /// in a ModVector object that has full type information.
    template <typename RETURN, typename... ARGS>
    struct ModVector : public ModVectorBase {

      /// Define the proper signal call type.
      typedef RETURN (ModuleBase::*ModMemFun)(ARGS...);

      /// Store the member-function call that this ModVector should handle.
      ModMemFun fun;

      /// A ModVector constructor takes both the member function that its supposed to call
      /// and a master list of module vectors that it should put itself it.
      ModVector(const std::string & _name,
                ModuleBase::SignalID _id,
                ModMemFun _fun,
                emp::vector< emp::Ptr< ModVectorBase > > & modv_ptrs)
        : ModVectorBase(_name, _id), fun(_fun)
      {
        modv_ptrs[id] = this;
      }

      template <typename... ARGS2>
      void Trigger(ARGS2 &&... args) {
        for (emp::Ptr<ModuleBase> mod_ptr : *this) {
          emp_assert(!mod_ptr.IsNull());
          (mod_ptr.Raw()->*fun)( std::forward<ARGS2>(args)... );
        }
      }

      template <typename... ARGS2>
      OrgPosition FindPosition(ARGS2 &&... args) {
        OrgPosition result;
        for (emp::Ptr<ModuleBase> mod_ptr : *this) {
          result = (mod_ptr.Raw()->*fun)(std::forward<ARGS2>(args)...);
          if (result.IsValid()) break;
        }
        return result;
      }
    };

    emp::vector<emp::Ptr<ModuleBase>> modules;  ///< Collection of ALL modules.

    // --- Track which modules need to have each signal type called on them. ---
    // BeforeUpdate(size_t update_ending)
    ModVector<void,size_t> before_update_sig;
    // OnUpdate(size_t new_update)
    ModVector<void,size_t> on_update_sig;
    // BeforeRepro(OrgPosition parent_pos) 
    ModVector<void,OrgPosition> before_repro_sig;
    // OnOffspringReady(Organism & offspring, OrgPosition parent_pos)
    ModVector<void,Organism &,OrgPosition> on_offspring_ready_sig;
    // OnInjectReady(Organism & inject_org)
    ModVector<void,Organism &> on_inject_ready_sig;
    // BeforePlacement(Organism & org, OrgPosition target_pos, OrgPosition parent_pos)
    ModVector<void,Organism &, OrgPosition, OrgPosition> before_placement_sig;
    // OnPlacement(OrgPosition placement_pos)
    ModVector<void,OrgPosition> on_placement_sig;
    // BeforeMutate(Organism & org)
    ModVector<void,Organism &> before_mutate_sig; // TO IMPLEMENT
    // OnMutate(Organism & org)
    ModVector<void,Organism &> on_mutate_sig; // TO IMPLEMENT
    // BeforeDeath(OrgPosition remove_pos)
    ModVector<void,OrgPosition> before_death_sig;
    // BeforeSwap(OrgPosition pos1, OrgPosition pos2)
    ModVector<void,OrgPosition,OrgPosition> before_swap_sig;
    // OnSwap(OrgPosition pos1, OrgPosition pos2)
    ModVector<void,OrgPosition,OrgPosition> on_swap_sig;
    // BeforePopResize(Population & pop, size_t new_size)
    ModVector<void,Population &, size_t> before_pop_resize_sig;
    // OnPopResize(Population & pop, size_t old_size)
    ModVector<void,Population &, size_t> on_pop_resize_sig;
    // OnError(const std::string & msg)
    ModVector<void,const std::string &> on_error_sig;
    // OnWarning(const std::string & msg)
    ModVector<void,const std::string &> on_warning_sig;
    // BeforeExit()
    ModVector<void> before_exit_sig;
    // OnHelp()
    ModVector<void> on_help_sig;

    // OrgPosition DoPlaceBirth(Organism &, OrgPosition);
    ModVector<OrgPosition, Organism &,OrgPosition> do_place_birth_sig;
    // OrgPosition DoPlaceInject(Organism &)
    ModVector<OrgPosition, Organism &> do_place_inject_sig;
    // OrgPosition DoFindNeighbor(OrgPosition) {
    ModVector<OrgPosition, OrgPosition> do_find_neighbor_sig;

    bool rescan_signals = true;   ///< Do module signals need to be updated?

    // Private constructor so that base class cannot be instantiated directly.
    MABEBase()
    : modv_ptrs(num_signals)
    , before_update_sig("before_update", ModuleBase::SIG_BeforeUpdate, &ModuleBase::BeforeUpdate, modv_ptrs)
    , on_update_sig("on_update", ModuleBase::SIG_OnUpdate, &ModuleBase::OnUpdate, modv_ptrs)
    , before_repro_sig("before_repro", ModuleBase::SIG_BeforeRepro, &ModuleBase::BeforeRepro, modv_ptrs)
    , on_offspring_ready_sig("on_offspring_ready", ModuleBase::SIG_OnOffspringReady, &ModuleBase::OnOffspringReady, modv_ptrs)
    , on_inject_ready_sig("on_inject_ready", ModuleBase::SIG_OnInjectReady, &ModuleBase::OnInjectReady, modv_ptrs)
    , before_placement_sig("before_placement", ModuleBase::SIG_BeforePlacement, &ModuleBase::BeforePlacement, modv_ptrs)
    , on_placement_sig("on_placement", ModuleBase::SIG_OnPlacement, &ModuleBase::OnPlacement, modv_ptrs)
    , before_mutate_sig("before_mutate", ModuleBase::SIG_BeforeMutate, &ModuleBase::BeforeMutate, modv_ptrs)
    , on_mutate_sig("on_mutate", ModuleBase::SIG_OnMutate, &ModuleBase::OnMutate, modv_ptrs)
    , before_death_sig("before_death", ModuleBase::SIG_BeforeDeath, &ModuleBase::BeforeDeath, modv_ptrs)
    , before_swap_sig("before_swap", ModuleBase::SIG_BeforeSwap, &ModuleBase::BeforeSwap, modv_ptrs)
    , on_swap_sig("on_swap", ModuleBase::SIG_OnSwap, &ModuleBase::OnSwap, modv_ptrs)
    , before_pop_resize_sig("before_pop_resize", ModuleBase::SIG_BeforePopResize, &ModuleBase::BeforePopResize, modv_ptrs)
    , on_pop_resize_sig("on_pop_resize", ModuleBase::SIG_OnPopResize, &ModuleBase::OnPopResize, modv_ptrs)
    , on_error_sig("on_error", ModuleBase::SIG_OnError, &ModuleBase::OnError, modv_ptrs)
    , on_warning_sig("on_warning", ModuleBase::SIG_OnWarning, &ModuleBase::OnWarning, modv_ptrs)
    , before_exit_sig("before_exit", ModuleBase::SIG_BeforeExit, &ModuleBase::BeforeExit, modv_ptrs)
    , on_help_sig("on_help", ModuleBase::SIG_OnHelp, &ModuleBase::OnHelp, modv_ptrs)
    , do_place_birth_sig("do_place_birth", ModuleBase::SIG_DoPlaceBirth, &ModuleBase::DoPlaceBirth, modv_ptrs)
    , do_place_inject_sig("do_place_inject", ModuleBase::SIG_DoPlaceInject, &ModuleBase::DoPlaceInject, modv_ptrs)
    , do_find_neighbor_sig("do_find_neighbor", ModuleBase::SIG_DoFindNeighbor, &ModuleBase::DoFindNeighbor, modv_ptrs)
    { ;  }

  public:

    /// Setup the signals to be rescanned; called this any time signal information is updated in
    /// a module.
    void RescanSignals() {
      rescan_signals = true;
    }

    /// All insertions of organisms should come through AddOrgAt
    /// Must provide an org_ptr that is now own by the population.
    /// Must specify the pos in the population to perform the insertion.
    /// Must specify parent position if it exists (for data tracking); not used with inject.
    void AddOrgAt(emp::Ptr<Organism> org_ptr, OrgPosition pos, OrgPosition ppos=OrgPosition()) {
      before_placement_sig.Trigger(*org_ptr, pos, ppos);
      ClearOrgAt(pos);      // Clear out any organism already in this position.
      pos.SetOrg(org_ptr);  // Put the new organism in place.
      on_placement_sig.Trigger(pos);
    }

    /// All permanent deletion of organisms from a population should come through here.
    void ClearOrgAt(OrgPosition pos) {
      emp_assert(pos.IsValid());
      if (pos.IsEmpty()) return; // Nothing to remove!

      before_death_sig.Trigger(pos);
      pos.ExtractOrg().Delete();
    }

    /// All movement of organisms from one population position to another should come through here.
    void SwapOrgs(OrgPosition pos1, OrgPosition pos2) {
      before_swap_sig.Trigger(pos1, pos2);
      emp::Ptr<Organism> org1 = pos1.ExtractOrg();
      emp::Ptr<Organism> org2 = pos2.ExtractOrg();
      if (!org1->IsEmpty()) pos2.SetOrg(org1);
      if (!org2->IsEmpty()) pos1.SetOrg(org2);
      on_swap_sig.Trigger(pos1, pos2);
    }

    /// Change the size of a population.  Clear orgs at removed positions; new positions should
    /// have empty organisms.
    void ResizePop(Population & pop, size_t new_size) {
      // Clean up any organisms that may be getting deleted.
      const size_t old_size = pop.GetSize();                // Track the starting size.
      if (old_size == new_size) return;                     // If size isn't changing, we're done!

      before_pop_resize_sig.Trigger(pop, new_size);

      for (size_t pos = new_size; pos < old_size; pos++) {  // Clear all orgs out of range.
        ClearOrgAt( OrgPosition(pop, pos) );
      }

      pop.Resize(new_size);                                 // Do the actual resize.

      on_pop_resize_sig.Trigger(pop, old_size);
    }

    /// Add a single, empty position on the end of a population.
    OrgPosition PushEmpty(Population & pop) {
      before_pop_resize_sig.Trigger(pop, pop.GetSize()+1);
      OrgPosition it = pop.PushEmpty();
      on_pop_resize_sig.Trigger(pop, pop.GetSize()-1);
      return it;
    }
  };

  /// The main MABE class that will be instantiated in the executable.
  class MABE : public MABEBase {
  private:
    const std::string VERSION = "0.0.1";

    emp::vector<Population> pops;      ///< Collection of populations.

    /// Collection of all organism types.
    emp::unordered_map<std::string, emp::Ptr<OrganismManagerBase>> org_managers;

    /// Collection of information about organism traits.
    std::unordered_map<std::string, emp::Ptr<TraitInfo>> trait_map;

    /// Trait information to be stored on each organism.
    emp::DataMap org_data_map;

    emp::Random random;                ///< Master random number generator
    int random_seed = 0;               ///< Random number seed.
    size_t cur_pop = (size_t) -1;      ///< Which population are we currently working with?
    size_t update = 0;                 ///< How many times has Update() been called?


    // --- Variables to handle configuration and initialization ---

    emp::vector<std::string> errors;   ///< Log any errors that have occured.
    bool show_help = false;            ///< Should we show "help" before exiting?
    bool exit_now = false;             ///< Do we need to immediately exit the code?

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

    emp::vector<ArgInfo> arg_set;              ///< Map of arguments to the 
    emp::vector<std::string> args;             ///< Command-line arguments passed in.
    emp::vector<std::string> config_filenames; ///< Names of configuration files
    std::string gen_filename;                  ///< Name of output file to generate.
    Config config;                             ///< Configutation information for this run.
    emp::Ptr<ConfigScope> cur_scope;           ///< Which config scope are we currently using?

    // ----------- Helper Functions -----------

    /// Call when ready to end a run.
    void Exit() {
      // Trigger functions
      before_exit_sig.Trigger();

      // Cleanup all pointers.
      for (auto [name,org_manager] : org_managers) org_manager.Delete();
      org_managers.clear();

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
      std::cout << "Note: parameter order matters. Settings and files are applied in the order provided.\n";
      Exit();
    }

    /// List out all of the available modules.
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

    void ProcessArgs();

    void Setup_Populations();
    void Setup_Modules();
    void Setup_Traits();

    void UpdateSignals();

  public:
    MABE(int argc, char* argv[]);  ///< MABE command-line constructor.
    MABE(const MABE &) = delete;
    MABE(MABE &&) = delete;
    ~MABE() {
      for (auto x : modules) x.Delete();
      for (auto [name,org_manager] : org_managers) org_manager.Delete();
      for (auto [name,trait_ptr] : trait_map) trait_ptr.Delete();
    }

    // --- Basic accessors ---

    emp::Random & GetRandom() { return random; }
    size_t GetUpdate() const noexcept { return update; }

    // --- Tools to setup runs ---
    bool Setup();

    /// Update MABE a single step.
    void Update();

    /// Update MABE the specified number of steps.
    void Update(size_t num_updates) {
      config.TriggerEvents("start");
      for (size_t ud = 0; ud < num_updates && !exit_now; ud++) {
        Update();
      }
    }

    // -- Error Handling --
    template <typename... Ts>
    void AddError(Ts &&... args) {
      errors.push_back( emp::to_string( std::forward<Ts>(args)... ));
      on_error_sig.Trigger(errors.back());
    }
    void AddErrors(const emp::vector<std::string> & in_errors) {
      errors.insert(errors.end(), in_errors.begin(), in_errors.end());
    }

    // -- World Structure --

    OrgPosition FindBirthPosition(Organism & org, OrgPosition ppos) {
      return do_place_birth_sig.FindPosition(org, ppos);
    }
    OrgPosition FindInjectPosition(Organism & org) {
      return do_place_inject_sig.FindPosition(org);
    }
    OrgPosition FindNeighbor(OrgPosition pos) {
      return do_find_neighbor_sig.FindPosition(pos);
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

    /// Move an organism from one position to another; kill anything that previously occupied
    /// the target position.
    void MoveOrg(OrgPosition from_pos, OrgPosition to_pos) {
      ClearOrgAt(to_pos);
      SwapOrgs(from_pos, to_pos);
    }

    /// Add the provided organism to the world.  Return the position injected; if more than one
    /// is added, return the position of the last one.
    OrgPosition Inject(const Organism & org, size_t copy_count=1) {
      OrgPosition pos;
      for (size_t i = 0; i < copy_count; i++) {
        emp::Ptr<Organism> inject_org = org.Clone();
        on_inject_ready_sig.Trigger(*inject_org);
        pos = FindInjectPosition(*inject_org);
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

    /// Add an organsim of a specified type to the world.
    OrgPosition Inject(const std::string & type_name, size_t copy_count=1) {      
      const auto & org_manager = GetOrganismManager(type_name);  // Look up type of organism.
      auto org_ptr = org_manager.MakeOrganism(random);           // Build an org of this type.
      OrgPosition pos = Inject(*org_ptr, copy_count);            // Inject a copy of the organism.
      org_ptr.Delete();                                          // Delete generated organism.
      return pos;                                                // Return last position injected.
    }

    /// Inject an organism at a specified position.
    void InjectAt(const Organism & org, OrgPosition pos) {
      emp_assert(pos.IsValid());
      emp::Ptr<Organism> inject_org = org.Clone();
      on_inject_ready_sig.Trigger(*inject_org);
      AddOrgAt( inject_org, pos);
    }

    /// Give birth to (potentially) multiple offspring; return position of last placed.
    /// Triggers 'before repro' signal on parent (once) and 'offspring ready' on each offspring.
    /// Regular signal triggers occur in AddOrgAt.
    OrgPosition DoBirth(const Organism & org, OrgPosition ppos, size_t copy_count=1) {
      emp_assert(org.IsEmpty() == false);  // Empty cells cannot reproduce.
      before_repro_sig.Trigger(ppos);
      OrgPosition pos;                                     // Position of each offspring placed.
      for (size_t i = 0; i < copy_count; i++) {            // Loop through offspring, adding each
        emp::Ptr<Organism> new_org = org.Clone();          // Clone org to put copy in population
        on_offspring_ready_sig.Trigger(*new_org, ppos);    // Trigger modules with offspring ready
        pos = FindBirthPosition(*new_org, ppos);           // Determine location for offspring

        if (pos.IsValid()) AddOrgAt(new_org, pos, ppos);   // If placement pos is valid, do so!
        else new_org.Delete();                             // Otherwise delete the organism.
      }
      return pos;
    }

    /// A shortcut to DoBirth where only the parent position needs to be supplied.
    OrgPosition Replicate(OrgPosition ppos, size_t copy_count=1) {
      return DoBirth(*ppos, ppos, copy_count);
    }

    /// Shortcut to resize a population of a specified id.
    void ResizePop(size_t pop_id, size_t new_size) {
      emp_assert(pop_id < pops.size());
      MABEBase::ResizePop(pops[pop_id], new_size);
    }

    /// Resize a population while clearing all of the organisms in it.
    void EmptyPop(Population & pop, size_t new_size) {
      // Clean up any organisms in the population.
      for (OrgPosition it = pop.begin_alive(); it != pop.end(); ++it) {
        ClearOrgAt(it);
      }

      MABEBase::ResizePop(pop, new_size);
    }

    /// Get a ramdom position from a desginated population.
    OrgPosition GetRandomPos(Population & pop) {
      emp_assert(pop.GetSize() > 0);
      return pop.IteratorAt( random.GetUInt(pop.GetSize()) );
    }

    /// Get a ramdom position from the population with the specified id.
    OrgPosition GetRandomPos(size_t pop_id) { return GetRandomPos(GetPopulation(pop_id)); }

    /// Get a ramdom position from a desginated population.
    OrgPosition GetRandomOrgPos(Population & pop) {
      emp_assert(pop.GetNumOrgs() > 0, "GetRandomOrgPos cannot be called if there are no orgs.");
      // @CAO: Something better to do in a sparse population?
      OrgPosition pos = GetRandomPos(pop);
      while (pos.IsEmpty()) pos = GetRandomPos(pop);
      return pos;
    }

    /// Get a ramdom position from the population with the specified id.
    OrgPosition GetRandomOrgPos(size_t pop_id) { return GetRandomOrgPos(GetPopulation(pop_id)); }

    // --- Module Management ---

    /// Get the unique id of a module with the specified name.
    int GetModuleID(const std::string & mod_name) const {
      return emp::FindEval(modules, [mod_name](const auto & m){ return m->GetName() == mod_name; });
    }

    /// Get a reference to a module with a specified ID.
    const ModuleBase & GetModule(int id) const { return *modules[(size_t) id]; }
    ModuleBase & GetModule(int id) { return *modules[(size_t) id]; }

    /// Add a module of the specified type.
    template <typename MOD_T, typename... ARGS>
    MOD_T & AddModule(ARGS &&... args) {
      auto new_mod = emp::NewPtr<MOD_T>(*this, std::forward<ARGS>(args)...);
      modules.push_back(new_mod);
      return *new_mod;
    }


    // --- Deal with Organism Types ---

    /// Get a reference to an organism manager with the specified name.
    OrganismManagerBase & GetOrganismManager(const std::string & type_name) {
      emp_assert(emp::Has(org_managers, type_name), type_name,
                 "An org type must be created before base retrieved.");
      return *(org_managers[type_name]);
    }

    /// Add a new organism manager with the specified information.
    template <typename ORG_TYPE_T>
    ORG_TYPE_T & AddOrganismManager(const std::string & type_name, const std::string & desc) {
      (void) desc; // @CAO: Do something with this!
      emp_assert(emp::Has(org_managers, type_name) == false);
      auto new_type = emp::NewPtr<ORG_TYPE_T>(type_name);
      org_managers[type_name] = new_type;
      return *new_type;
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


    // --- Manage configuration scope ---

    /// Access to the current configuration scope.
    ConfigScope & GetCurScope() { return *cur_scope; }

    /// Add a new scope under the current one.
    ConfigScope & PushScope(const std::string & name, const std::string & desc) {
      cur_scope = &(cur_scope->AddScope(name, desc));
      return *cur_scope;
    }

    /// Move up one level of scope.
    ConfigScope & PopScope() {
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

    /// Check to make sure that all details of this MABE setup are "okay".
    bool OK();
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

    // Setup all modules as types in the config file.
    for (auto & mod : GetModuleInfo()) {
      std::function<ConfigType &(const std::string &)> mod_init_fun =
        [this,&mod](const std::string & name) -> ConfigType & {
          return mod.init_fun(*this,name);
        };
      config.AddType(mod.name, mod.desc, mod_init_fun);
    }

    // Setup all organism types as types in the config file.
    for (auto & org_m : GetOrgManagerInfo()) {
      std::function<ConfigType &(const std::string &)> org_m_init_fun =
        [this,&org_m](const std::string & name) -> ConfigType & {
          return org_m.init_fun(*this,name);
        };
      config.AddType(org_m.name, org_m.desc, org_m_init_fun);
    }


    // Add in other built-in functions.
    std::function<int()> exit_fun = [this](){ Exit(); return 0; };
    config.AddFunction("exit", exit_fun, "Exit from this MABE run.");

    std::function<int(const std::string, size_t)> inject_fun =
      [this](const std::string org_type_name, size_t count) {
        Inject(org_type_name, count);
        return 0;
      };
    config.AddFunction("inject", inject_fun,
      "Inject organisms into a population (args: org_name, org_count).");

    std::function<int(const emp::vector<emp::Ptr<ConfigEntry>> &)> print_fun =
      [](const emp::vector<emp::Ptr<ConfigEntry>> & args) {
        for (auto entry_ptr : args) std::cout << entry_ptr->AsString();
        return 0;
      };
    config.AddFunction("print", print_fun, "Print out the provided variable.");

    // Add in other built-in events.
    config.AddEventType("start");   // Triggered at the beginning of a run.
    config.AddEventType("update");  // Tested every update.
  }

  bool MABE::Setup() {
    SetupConfig();                   // Load all of the parameters needed by modules, etc.
    ProcessArgs();                   // Deal with command-line inputs.
    if (exit_now) return false;

    // If configuration filenames have been specified, load each of them in order.
    if (config_filenames.size()) {
      std::cout << "Loading file(s): " << emp::to_quoted_list(config_filenames) << std::endl;
      config.Load(config_filenames);   // Load files
    }

    // If we are writing a file, do so and then stop.
    if (gen_filename != "") {
      std::cout << "Generating file '" << gen_filename << "'." << std::endl;
      config.Write(gen_filename);
      Exit();
    }

    // If any of the inital flags triggered an 'exit_now', do so.
    if (exit_now) return false;

    Setup_Populations();    // Give modules access to the correct populations.
    Setup_Modules();        // Run SetupModule() on each module; initialize placement if needed.
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
          std::cout << "--generate must be followed by a single filename.\n";
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

  void MABE::Setup_Populations() {
    // Now loop through the modules and make sure all populations are assigned.
    for (emp::Ptr<ModuleBase> mod_ptr : modules) {
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
    for (emp::Ptr<ModuleBase> mod_ptr : modules) mod_ptr->SetupModule();

    // @CAO: If no modules are marked IsPlacementMod(), as a new final module.
  }

  void MABE::Setup_Traits() {
    // STEP 1: Make sure modules are accessing traits correctly and consistently.

    int error_count = 0;

    // Loop through all of the traits to ensure there are no conflicts.
    for (auto [trait_name, trait_ptr] : trait_map) {
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

      // A trait that is OWNED cannot have other modules writing to it.
      else if (trait_ptr->GetOwnedCount() > 1) {
        std::stringstream error_msg;
        error_msg << "Multiple modules declaring ownership of trait '" << trait_name << "': "
                  << emp::to_english_list(trait_ptr->GetOwnedNames()) << ".\n"
                  << "[Suggestion: if traits are supposed to be distinct, prepend names with a\n"
                  << " module-specific prefix.  Otherwise modules should be edited to change trait\n"
                  << " to be SHARED (and all can modify) or have all but one shift to REQUIRED.]";
        AddError(error_msg.str());
        error_count++;
        continue;
      }

      else if (trait_ptr->IsOwned() && trait_ptr->IsShared()) {
        AddError("Trait '", trait_name, "' is fully OWNED by module '", trait_ptr->GetOwnedNames()[0],
                 "'; it cannot be SHARED (written to) by other modules:",
                 emp::to_english_list(trait_ptr->GetSharedNames()),
                 "[Suggestion: if traits are supposed to be distinct, prepend private name with a\n",
                 " module-specific prefix.  Otherwise module needs to be edited to make trait\n",
                 " SHARED or have all but one shift to REQUIRED.]");
        error_count++;
        continue;
      }

      // A trait that is REQUIRED must have another module write to it (i.e. be OWNED or SHARED).
      else if (trait_ptr->IsRequired() && !trait_ptr->IsOwned() && !trait_ptr->IsShared()) {
        AddError("Trait '", trait_name, "' marked REQUIRED by module(s) ",
                 emp::to_english_list(trait_ptr->GetRequiredNames()),
                 "'; must be written to by other modules.\n",
                 "[Suggestion: set another module to write to this trait (where it is either\n",
                 " SHARED or OWNED).]");
        error_count++;
        continue;
      }
    }

    if (error_count == 0) {
      std::cout << "Organism traits are initialized; no conflicts found." << std::endl;
    }
  }

  /// Link signals to the modules that implment responses to those signals.
  void MABE::UpdateSignals() {
    // Clear all module vectors.
    for (auto modv : modv_ptrs) modv->resize(0);

    // Loop through each module to update its signals.
    for (emp::Ptr<ModuleBase> mod_ptr : modules) {
      // For the current module, loop through all of the signals.
      for (size_t sig_id = 0; sig_id < num_signals; sig_id++) {
        if (mod_ptr->has_signal[sig_id]) modv_ptrs[sig_id]->push_back(mod_ptr);
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
                        "Seed for random number generator; use 0 to base on time.",
                        0).SetMin(0);

    // Call the SetupConfig of module base classes (they will call the dervived version)
    for (auto m : modules) {
      PushScope(m->GetName(), m->GetDesc());
      m->SetupScope(*cur_scope);
      m->SetupConfig();
      PopScope();
    }

    // Loop through organism types.
    for (auto o : org_managers) {
      PushScope(o.first, "Organism type");
      o.second->SetupScope(*cur_scope);
      o.second->SetupConfig();
      PopScope();
    }
  }


  bool MABE::OK() {
    bool result = true;

    // Make sure the populations are all OK.
    for (size_t pop_id = 0; pop_id < pops.size(); pop_id++) {
      result &= pops[pop_id].OK();
    }

    // @CAO: Should check to make sure modules and organism managers are okay too.

    return result;
  }

}

#endif
