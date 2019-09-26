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

#include "ModuleBase.h"
#include "Population.h"

namespace mabe {

  /// MABEBase sets up all population-manipulation functionality (nowhere else can alter a
  /// Population object).  As such, it guarantees that all manipulation calls ultimately
  /// come through the limited functions defined here.

  class MABEBase {
  protected:
    /// ModVectors should be derived from a vector of module pointer that they operator on.
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
    /// in a ModVector object.
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

    OrgPosition PushEmpty(Population & pop) {
      before_pop_resize_sig.Trigger(pop, pop.GetSize()+1);
      OrgPosition it = pop.PushEmpty();
      on_pop_resize_sig.Trigger(pop, pop.GetSize()-1);
      return it;
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

    size_t cur_pop = (size_t) -1;           ///< Which population are we currently working with?
    size_t update = 0;                      ///< How many times has Update() been called?
    emp::vector<std::string> errors;        ///< Log any errors that have occured.

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
    emp::Ptr<ConfigScope> cur_scope;           ///< Which config scope are we currently using?

    // ----------- Helper Functions -----------

    void Exit() {
      // Trigger functions
      before_exit_sig.Trigger();

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
      on_help_sig.Trigger();
      std::cout << "Note: parameter order matters. Settings and files are applied in the order provided.\n";
      Exit();
    }

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
    MABE(int argc, char* argv[])
      : args(emp::cl::args_to_strings(argc, argv))
      , cur_scope(&(config.GetRootScope()))
    {
      // Setup "Population" as a type in the config file.
      std::function<ConfigType &(const std::string &)> pop_init_fun =
        [this](const std::string & name) -> ConfigType & {
          return AddPopulation(name);
        };
      config.AddType("Population", pop_init_fun);

      // Setup all modules as types in the config file.
      for (auto & mod : GetModuleInfo()) {
        std::function<ConfigType &(const std::string &)> mod_init_fun =
          [this,&mod](const std::string & name) -> ConfigType & {
            return mod.init_fun(*this,name);
          };
        config.AddType(mod.name, mod_init_fun);
      }
    }
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
      SetupConfig();                   // Load all of the parameters needed by modules, etc.
      ProcessArgs();                   // Deal with command-line inputs.
      config.Load(config_filenames);   // Load files, if any.

      // If we are writing a file, do so and stop.
      if (gen_filename != "") { config.Write(gen_filename); Exit(); }

      Setup_Populations();    // Give modules access to the correct populations.
      Setup_Modules();        // Run SetupModule() on each module; initialize placement if needed.
      Setup_Traits();         // Make sure module traits do not clash.

      UpdateSignals();        // Setup the appropriate modules to be linked with each signal.

      // Collect errors in any module.
      for (emp::Ptr<ModuleBase> mod_ptr : modules) {
        if (mod_ptr->HasErrors()) { AddErrors(mod_ptr->GetErrors()); }
      }
    }

    /// Update MABE a single step.
    void Update() {
      emp_assert(OK(), update);

      // If informaiton on any of the signals has changed, update them.
      if (rescan_signals) UpdateSignals();

      // Signal that a new update is about to begin.
      before_update_sig.Trigger(update);

      // Increment 'update' to start new update.
      update++;

      // Run Update on all modules...
      on_update_sig.Trigger(update);
    }

    /// Update MABE the specified number of steps.
    void Update(size_t num_updates) {
      for (size_t ud = 0; ud < num_updates; ud++) {
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

    void MoveOrg(OrgPosition from_pos, OrgPosition to_pos) {
      ClearOrgAt(to_pos);
      SwapOrgs(from_pos, to_pos);
    }

    void Inject(const Organism & org, size_t copy_count=1) {
      for (size_t i = 0; i < copy_count; i++) {
        emp::Ptr<Organism> inject_org = org.Clone();
        on_inject_ready_sig.Trigger(*inject_org);
        OrgPosition pos = FindInjectPosition(*inject_org);
        if (pos.IsValid()) AddOrgAt( inject_org, pos);
        else {
          inject_org.Delete();
          AddError("Invalid position (pop=", pos.PopPtr(), "; pos=", pos.Pos(),
                   "); failed to inject organism ", i, "!");
        }
      }
    }

    void Inject(const std::string & type_name, size_t copy_count=1) {      
      const OrganismManager & org_manager = GetOrganismManager(type_name);
      emp::Ptr<Organism> inject_org = org_manager.MakeOrganism(random);
      Inject(*inject_org, copy_count);
      inject_org.Delete();
    }

    void InjectAt(const Organism & org, OrgPosition pos) {
      emp_assert(pos.IsValid());
      emp::Ptr<Organism> inject_org = org.Clone();
      on_inject_ready_sig.Trigger(*inject_org);
      AddOrgAt( inject_org, pos);
    }

    // Give birth to (potentially) multiple offspring; return position of last placed.
    // Triggers 'before repro' signal on parent (once) and 'offspring ready' on each offspring.
    // Regular signal triggers occur in AddOrgAt.
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

    /// Shortcut to resize a population by id.
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

    const ModuleBase & GetModule(int id) const { return *modules[(size_t) id]; }
    ModuleBase & GetModule(int id) { return *modules[(size_t) id]; }

    template <typename MOD_T, typename... ARGS>
    MOD_T & AddModule(ARGS &&... args) {
      auto mod_ptr = emp::NewPtr<MOD_T>(*this, std::forward<ARGS>(args)...);
      modules.push_back(mod_ptr);

      return *mod_ptr;
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

    /// Setup the configuration options for MABE.
    void SetupConfig() {
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
        o.second->SetupConfig(*this);
        PopScope();
      }
    }


    // ------ DEBUG FUNCTIONS -----
    bool OK() {
      bool result = true;

      // Make sure the populations are all OK.
      for (size_t pop_id = 0; pop_id < pops.size(); pop_id++) {
        result &= pops[pop_id].OK();
      }

      return result;
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

    // If none of the modules setup the placement functions, do so now.
    // @CAO: If no modules are marked IsPlacementMod(), make that the last module.
  }

  void MABE::Setup_Traits() {

  }
  
  // Function to link signals to the modules that implment responses to those signals.
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

}

#endif
