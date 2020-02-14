/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  ModuleBase.h
 *  @brief Base class for all MABE modules; does not have access to MABE object (Module does)
 * 
 *  Development Notes
 *  - Various On* and Before* functions should be automatically detected and run when relevant.
 *    These include:
 *     BeforeUpdate(size_t update_ending)
 *       : Update is ending; new one is about to start
 *     OnUpdate(size_t new_update)
 *       : New update has just started.
 *     BeforeRepro(OrgPosition parent_pos) 
 *       : Parent is about to reporduce.
 *     OnOffspringReady(Organism & offspring, OrgPosition parent_pos)
 *       : Offspring is ready to be placed.
 *     OnInjectReady(Organism & inject_org)
 *       : Organism to be injected is ready to be placed.
 *     BeforePlacement(Organism & org, OrgPosition target_pos)
 *       : Placement location has been identified (For birth or inject)
 *     OnPlacement(OrgPosition placement_pos)
 *       : New organism has been placed in the poulation.
 *     BeforeMutate(Organism & org)
 *       : Mutate is about to run on an organism.
 *     OnMutate(Organism & org)
 *       : Organism has had its genome changed due to mutation.
 *     BeforeDeath(OrgPosition remove_pos)
 *       : Organism is about to die.
 *     BeforeSwap(OrgPosition pos1, OrgPosition pos2)
 *       : Two organisms' positions in the population are about to move.
 *     OnSwap(OrgPosition pos1, OrgPosition pos2)
 *       : Two organisms' positions in the population have just swapped.
 *     BeforePopResize(Population & pop, size_t new_size)
 *       : Full population is about to be resized.
 *     OnPopResize(Population & pop, size_t old_size)
 *       : Full population has just been resized.
 *     OnError(const std::string & msg)
 *       : An error has occurred and the user should be notified.
 *     OnWarning(const std::string & msg)
 *       : A atypical condition has occurred and the user should be notified.
 *     BeforeExit()
 *       : Run immediately before MABE is about to exit.
 *     OnHelp()
 *       : Run when the --help option is called at startup.
 *     ...
 * 
 *    - Various Do* functions run in modules until one of them returns a valid answer.
 *     DoPlaceBirth(Organism & offspring, OrgPosition parent position)
 *       : Place a new offspring about to be born.
 *     DoPlaceInject(Organism & new_org)
 *       : Place a new offspring about to be injected.
 *     DoFindNeighbor(OrgPosition target_pos)
 *       : Find a random neighbor to a designated position.
 */

#ifndef MABE_MODULE_BASE_H
#define MABE_MODULE_BASE_H

#include <set>
#include <string>

#include "base/map.h"
#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/map_utils.h"
#include "tools/reference_vector.h"

#include "../config/Config.h"

#include "TraitInfo.h"

namespace mabe {

  class MABE;
  class Organism;
  class OrgPosition;
  class Population;

  class ModuleBase : public mabe::ConfigType {
    friend MABE;
  protected:
    std::string name;                 ///< Unique name for this module.
    std::string desc;                 ///< Description for this module.
    mabe::MABE & control;             ///< Reference to main mabe controller using this module
    emp::vector<std::string> errors;  ///< Has this class detected any configuration errors?

    /// Informative tags about this module.  Expected tags include:
    ///   "Analyze"     : Records data or makes measurements on the population.
    ///   "ErrorHandle" : Deals with errors as they occur and need to be reported.
    ///   "Evaluate"    : Examines organisms and annotates the data map.
    ///   "Interface"   : Provides mechanisms for the user to interact with the world.
    ///   "ManageOrgs"  : Manages a type of organism in the world.
    ///   "Mutate"      : Modifies organism genomes
    ///   "Placement"   : Identifies where new organisms should be placed in the population.
    ///   "Select"      : Chooses organisms to act as parents in for the next generation.
    ///   "Visualize"   : Displays data for the user.
    std::set<std::string> action_tags; ///< Informative tags about this model

    /// How many populations are we operating on?
    size_t min_pops = 0;                           ///< Minimum number of population needed

    /// Set of traits that this module is working with.
    emp::map<std::string, emp::Ptr<TraitInfo>> trait_map;

    /// Other variables that we want to hook on to this Module externally.
    emp::DataMap data_map;

    /// If this module is an organism manager, maintain a prototype of the organisms it handles.
    emp::Ptr<Organism> org_prototype;  ///< Base organism to copy.

  public:
    // Setup each signal with a unique ID number
    enum SignalID {
      SIG_BeforeUpdate = 0,
      SIG_OnUpdate,
      SIG_BeforeRepro,
      SIG_OnOffspringReady,
      SIG_OnInjectReady,
      SIG_BeforePlacement,
      SIG_OnPlacement,
      SIG_BeforeMutate,
      SIG_OnMutate,
      SIG_BeforeDeath,
      SIG_BeforeSwap,
      SIG_OnSwap,
      SIG_BeforePopResize,
      SIG_OnPopResize,
      SIG_OnError,
      SIG_OnWarning,
      SIG_BeforeExit,
      SIG_OnHelp,
      SIG_DoPlaceBirth,
      SIG_DoPlaceInject,
      SIG_DoFindNeighbor,
      NUM_SIGNALS,
      SIG_UNKNOWN
    };

  protected:
    // Setup a BitSet to track if this module has each signal implemented.
    emp::BitSet<NUM_SIGNALS> has_signal;

    // ---- Helper functions ----

    /// All internal errors should be processed through AddError(...)
    template <typename... Ts>
    void AddError(Ts &&... args) {
      errors.push_back( emp::to_string( std::forward<Ts>(args)... ));
      std::cerr << "ERROR: " << errors.back() << std::endl;
    }

  public:
    ModuleBase(MABE & in_control, const std::string & in_name, const std::string & in_desc="")
      : name(in_name), desc(in_desc), control(in_control)
    {
      has_signal.SetAll(); // Default all signals to on until base class version is run.
    }
    ModuleBase(const ModuleBase &) = default;
    ModuleBase(ModuleBase &&) = default;
    virtual ~ModuleBase() {
      // Clean up trait information.
      for (auto & x : trait_map) x.second.Delete();
    }

    const std::string & GetName() const noexcept { return name; }
    const std::string & GetDesc() const noexcept { return desc; }
    bool HasErrors() const { return errors.size(); }
    const emp::vector<std::string> & GetErrors() const noexcept { return errors; }
    size_t GetMinPops() const noexcept { return min_pops; }

    virtual std::string GetTypeName() const { return "ModuleBase"; }
    virtual emp::Ptr<ModuleBase> Clone() { return nullptr; }

    bool IsAnalyzeMod() const { return emp::Has(action_tags, "Analyze"); }
    bool IsErrorHandleMod() const { return emp::Has(action_tags, "ErrorHandle"); }
    bool IsEvaluateMod() const { return emp::Has(action_tags, "Evaluate"); }
    bool IsInterfaceMod() const { return emp::Has(action_tags, "Interface"); }
    bool IsManageMod() const { return emp::Has(action_tags, "ManageOrgs"); }
    bool IsMutateMod() const { return emp::Has(action_tags, "Mutate"); }
    bool IsPlacementMod() const { return emp::Has(action_tags, "Placement"); }
    bool IsSelectMod() const { return emp::Has(action_tags, "Select"); }
    bool IsVisualizeMod() const { return emp::Has(action_tags, "Visualize"); }

    ModuleBase & SetActionTag(const std::string & name, bool setting=true) {
      if (setting) action_tags.insert(name);
      else action_tags.erase(name);
      return *this;
    }

    ModuleBase & SetAnalyzeMod(bool in=true) { return SetActionTag("Analyze", in); }
    ModuleBase & SetErrorHandleMod(bool in=true) { return SetActionTag("ErrorHandle", in); }
    ModuleBase & SetEvaluateMod(bool in=true) { return SetActionTag("Evaluate", in); }
    ModuleBase & SetInterfaceMod(bool in=true) { return SetActionTag("Interface", in); }
    ModuleBase & SetManageMod(bool in=true) { return SetActionTag("ManageOrgs", in); }
    ModuleBase & SetMutateMod(bool in=true) { return SetActionTag("Mutate", in); }
    ModuleBase & SetPlacementMod(bool in=true) { return SetActionTag("Placement", in); }
    ModuleBase & SetSelectMod(bool in=true) { return SetActionTag("Select", in); }
    ModuleBase & SetVisualizerMod(bool in=true) { return SetActionTag("Visualize", in); }

    virtual void SetupModule() { /* By default, assume no setup needed. */ }

    // ----==== SIGNALS ====----

    // Base classes for signals to be called (More details in Module.h)

    virtual void BeforeUpdate(size_t) = 0;
    virtual void OnUpdate(size_t) = 0;
    virtual void BeforeRepro(OrgPosition) = 0;
    virtual void OnOffspringReady(Organism &, OrgPosition) = 0;
    virtual void OnInjectReady(Organism &) = 0;
    virtual void BeforePlacement(Organism &, OrgPosition, OrgPosition) = 0;
    virtual void OnPlacement(OrgPosition) = 0;
    virtual void BeforeMutate(Organism &) = 0;
    virtual void OnMutate(Organism &) = 0;
    virtual void BeforeDeath(OrgPosition) = 0;
    virtual void BeforeSwap(OrgPosition, OrgPosition) = 0;
    virtual void OnSwap(OrgPosition, OrgPosition) = 0;
    virtual void BeforePopResize(Population &, size_t) = 0;
    virtual void OnPopResize(Population &, size_t) = 0;
    virtual void OnError(const std::string &) = 0;
    virtual void OnWarning(const std::string &) = 0;
    virtual void BeforeExit() = 0;
    virtual void OnHelp() = 0;

    virtual OrgPosition DoPlaceBirth(Organism &, OrgPosition) = 0;
    virtual OrgPosition DoPlaceInject(Organism &) = 0;
    virtual OrgPosition DoFindNeighbor(OrgPosition) = 0;

    virtual void Deactivate() = 0;  ///< Turn off all signals in this function.
    virtual void Activate() = 0;    ///< Turn on all signals in this function.

    // ---=== Specialty Functions for Organism Managers ===---
    virtual emp::TypeID GetOrgType() const {
      emp_assert(false, "GetOrgType() must be overridden for either Organism or OrganismManager module.");
      return emp::TypeID();
    }
    virtual emp::Ptr<Organism> CloneOrganism(const Organism &) const {
      emp_assert(false, "CloneOrganism() must be overridden for either Organism or OrganismManager module.");
      return nullptr;
    }
    virtual emp::Ptr<Organism> CloneOrganism(const Organism &, emp::Random &) const {
      emp_assert(false, "CloneOrganism() must be overridden for either Organism or OrganismManager module.");
      return nullptr;
    }
    virtual emp::Ptr<Organism> MakeOrganism() const {
      emp_assert(false, "MakeOrganism() must be overridden for either Organism or OrganismManager module.");
      return nullptr;
    }
    virtual emp::Ptr<Organism> MakeOrganism(emp::Random &) const {
      emp_assert(false, "MakeOrganism() must be overridden for either Organism or OrganismManager module.");
      return nullptr;
    }
    virtual std::string OrgToString(const Organism &) const {
      emp_assert(false, "OrgToString() must be overridden for either Organism or OrganismManager module.");
      return "";
    }
    virtual std::ostream & PrintOrganism(Organism &, std::ostream & is) const {
      emp_assert(false, "Print() must be overridden for either Organism or OrganismManager module.");
      return is;
    }
    virtual size_t Mutate(Organism &, emp::Random &) const {
      emp_assert(false, "Mutate() must be overridden for either Organism or OrganismManager module.");
      return 0;
    }
    virtual void Randomize(Organism &, emp::Random &) const {
      emp_assert(false, "Randomize() must be overridden for either Organism or OrganismManager module.");
    }

    virtual void SetupConfig() { }

  // --------------------- Functions to be used in derived modules ONLY --------------------------
  protected:

    /// Set the number of populations that this module must work on.
    void SetMinPops(size_t in_min) { min_pops = in_min; }

  };

  struct ModuleInfo {
    std::string name;
    std::string desc;
    std::function<ConfigType & (MABE &, const std::string &)> init_fun;
    bool operator<(const ModuleInfo & in) const { return name < in.name; }
  };

  static std::set<ModuleInfo> & GetModuleInfo() {
    static std::set<ModuleInfo> mod_type_info;
    return mod_type_info;
  }

  static void PrintModuleInfo() {
    auto & mod_info = GetModuleInfo();
    for (auto & mod : mod_info) {
      std::cout << mod.name << " : " << mod.desc << std::endl;
    }
  }
}

#endif
