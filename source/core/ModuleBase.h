/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
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

#include <string>

#include "base/map.h"
#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/map_utils.h"
#include "tools/reference_vector.h"

#include "../config/Config.h"

#include "Population.h"
#include "TraitInfo.h"

namespace mabe {

  class MABE;

  class ModuleBase {
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

    /// Is this module expecting sychronous replication (i.e., discrete generations) or
    /// asynchronous replication (i.e., overlapping generations)?  The former is more common
    /// in evolutionary computation, while the latter is more common in artificial life.
    /// Modules with no-preference will be ignored.  Modules with a requirement will force
    /// the replication type (and give an error if requirements contradict each other).
    /// Otherwise the more common default will be used, with synchronous used in case of a tie.
    enum class ReplicationType {
      NO_PREFERENCE=0, REQUIRE_ASYNC, DEFAULT_ASYNC, DEFAULT_SYNC, REQUIRE_SYNC
    };
    ReplicationType rep_type = ReplicationType::NO_PREFERENCE;

    /// How many populations are we operating on?
    size_t min_pops = 0;                           ///< Minimum number of population needed

    /// Which traits is this module working with?
    emp::map<std::string, emp::Ptr<TraitInfo>> trait_map;

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

    ModuleBase & RequireAsync() { rep_type = ReplicationType::REQUIRE_ASYNC; return *this; }
    ModuleBase & DefaultAsync() { rep_type = ReplicationType::DEFAULT_ASYNC; return *this; }
    ModuleBase & DefaultSync() { rep_type = ReplicationType::DEFAULT_SYNC; return *this; }
    ModuleBase & RequireSync() { rep_type = ReplicationType::REQUIRE_SYNC; return *this; }

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

  // --------------------- Functions to be used in derived modules ONLY --------------------------
  protected:

    /// Set the number of populations that this module must work on.
    void SetMinPops(size_t in_min) { min_pops = in_min; }

    // ---== Trait management ==---
   
    /// Add a new trait to this module, specifying its access method, its name, and its description.
    template <typename T>
    TypedTraitInfo<T> & AddTrait(TraitInfo::Access access,
                                 const std::string & in_name,
                                 const std::string & desc="") {
      if (emp::Has(trait_map, in_name)) {
        AddError("Module ", name, " is creating a duplicate trait named '", in_name, "'.");
      }
      auto new_ptr = emp::NewPtr<TypedTraitInfo<T>>(in_name);
      new_ptr->SetAccess(access).SetOwner(this).SetDescription(desc);
      trait_map[in_name] = new_ptr;
      return *new_ptr;
    }

    /// Add a new trait to this module, specifying its access method, its name, and its description
    /// AND its default value.
    template <typename T>
    TypedTraitInfo<T> & AddTrait(TraitInfo::Access access,
                                 const std::string & name,
                                 const std::string & desc,
                                 const T & default_val) {
      return AddTrait<T>(access, name, desc).SetDefault(default_val);
    }

    /// Add trait that this module can READ & WRITE this trait.  Others cannot use it.
    /// Must provide name, description, and a default value to start at.
    template <typename T>
    TraitInfo & AddPrivateTrait(const std::string & name, const std::string & desc, const T & default_val) {
      return AddTrait<T>(TraitInfo::Access::PRIVATE, name, desc, default_val);
    }

    /// Add trait that this module can READ & WRITE to; other modules can only read.
    /// Must provide name, description, and a default value to start at.
    template <typename T>
    TraitInfo & AddOwnedTrait(const std::string & name, const std::string & desc, const T & default_val) {
      return AddTrait<T>(TraitInfo::Access::OWNED, name, desc, default_val);
    }
   
    /// Add trait that this module can READ & WRITE this trait; other modules can too.
    /// Must provide name, description; a default value is optional, but at least one
    /// module MUST set and it must be consistent across all modules that use it.
    template <typename T>
    TraitInfo & AddSharedTrait(const std::string & name, const std::string & desc="") {
      return AddTrait<T>(TraitInfo::Access::SHARED, name, desc);
    }
    template <typename T>
    TraitInfo & AddSharedTrait(const std::string & name, const std::string & desc, const T & default_val) {
      return AddTrait<T>(TraitInfo::Access::SHARED, name, desc, default_val);
    }
   
    /// Add trait that this module can READ this trait, but another module must WRITE to it.
    /// That other module should also provide the description for the trait.
    template <typename T>
    TraitInfo & AddRequiredTrait(const std::string & name) {
      return AddTrait<T>(TraitInfo::Access::REQUIRED, name);
    }

    
    // ---==  Configuration Management ==---

    /// Setup the module-specific configuration options.
    virtual void SetupConfig() { ; }

  };

  static emp::vector<Config::TypeInfo> & GetModuleTypeInfo() {
    static emp::vector<Config::TypeInfo> mod_type_info;
    return mod_type_info;
  }

}

#endif
