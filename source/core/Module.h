/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Module.h
 *  @brief Base class for all MABE modules.
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
 */

#ifndef MABE_MODULE_H
#define MABE_MODULE_H

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

  class Module {
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

    // Helper functions
    template <typename... Ts>
    void AddError(Ts &&... args) {
      errors.push_back( emp::to_string( std::forward<Ts>(args)... ));
      std::cerr << "ERROR: " << errors.back() << std::endl;
    }

  public:
    Module(MABE & in_control, const std::string & in_name, const std::string & in_desc="")
      : name(in_name), desc(in_desc), control(in_control) { ; }
    Module(const Module &) = default;
    Module(Module &&) = default;
    virtual ~Module() {
      // Clean up trait information.
      for (auto & x : trait_map) x.second.Delete();
    }

    const std::string & GetName() const noexcept { return name; }
    const std::string & GetDesc() const noexcept { return desc; }
    bool HasErrors() const { return errors.size(); }
    const emp::vector<std::string> & GetErrors() const noexcept { return errors; }
    size_t GetMinPops() const noexcept { return min_pops; }

    virtual emp::Ptr<Module> Clone() { return nullptr; }

    bool IsAnalyzeMod() const { return emp::Has(action_tags, "Analyze"); }
    bool IsErrorHandleMod() const { return emp::Has(action_tags, "ErrorHandle"); }
    bool IsEvaluateMod() const { return emp::Has(action_tags, "Evaluate"); }
    bool IsInterfaceMod() const { return emp::Has(action_tags, "Interface"); }
    bool IsManageMod() const { return emp::Has(action_tags, "ManageOrgs"); }
    bool IsMutateMod() const { return emp::Has(action_tags, "Mutate"); }
    bool IsPlacementMod() const { return emp::Has(action_tags, "Placement"); }
    bool IsSelectMod() const { return emp::Has(action_tags, "Select"); }
    bool IsVisualizeMod() const { return emp::Has(action_tags, "Visualize"); }

    Module & SetActionTag(const std::string & name, bool setting=true) {
      if (setting) action_tags.insert(name);
      else action_tags.erase(name);
      return *this;
    }

    Module & SetAnalyzeMod(bool in=true) { return SetActionTag("Analyze", in); }
    Module & SetErrorHandleMod(bool in=true) { return SetActionTag("ErrorHandle", in); }
    Module & SetEvaluateMod(bool in=true) { return SetActionTag("Evaluate", in); }
    Module & SetInterfaceMod(bool in=true) { return SetActionTag("Interface", in); }
    Module & SetManageMod(bool in=true) { return SetActionTag("ManageOrgs", in); }
    Module & SetMutateMod(bool in=true) { return SetActionTag("Mutate", in); }
    Module & SetPlacementMod(bool in=true) { return SetActionTag("Placement", in); }
    Module & SetSelectMod(bool in=true) { return SetActionTag("Select", in); }
    Module & SetVisualizerMod(bool in=true) { return SetActionTag("Visualize", in); }

    Module & RequireAsync() { rep_type = ReplicationType::REQUIRE_ASYNC; return *this; }
    Module & DefaultAsync() { rep_type = ReplicationType::DEFAULT_ASYNC; return *this; }
    Module & DefaultSync() { rep_type = ReplicationType::DEFAULT_SYNC; return *this; }
    Module & RequireSync() { rep_type = ReplicationType::REQUIRE_SYNC; return *this; }

    virtual void SetupModule() { /* By default, assume no setup needed. */ }

    // Functions to be called based on signals.  Note that the existance of an overridden version
    // of each function is tracked by an associated bool value that we default to true until the
    // base version of the function is called.

    // Format:  BeforeUpdate(size_t update_ending)
    // Trigger: Update is ending; new one is about to start
    bool has_BeforeUpdate = true;
    virtual void BeforeUpdate(size_t) { has_BeforeUpdate = false; }    

    // Format:  OnUpdate(size_t new_update)
    // Trigger: New update has just started.
    bool has_OnUpdate = true;
    virtual void OnUpdate(size_t) { has_OnUpdate = false; }    

    // Format:  BeforeRepro(OrgPosition parent_pos) 
    // Trigger: Parent is about to reproduce.
    bool has_BeforeRepro = true;
    virtual void BeforeRepro(OrgPosition) { has_BeforeRepro = false; }    

    // Format:  OnOffspringReady(Organism & offspring, OrgPosition parent_pos)
    // Trigger: Offspring is ready to be placed.
    bool has_OnOffspringReady = true;
    virtual void OnOffspringReady(Organism &, OrgPosition) { has_OnOffspringReady = false; }    

    // Format:  OnInjectReady(Organism & inject_org)
    // Trigger: Organism to be injected is ready to be placed.
    bool has_OnInjectReady = true;
    virtual void OnInjectReady(Organism &) { has_OnInjectReady = false; }    

    // Format:  BeforePlacement(Organism & org, OrgPosition target_pos)
    // Trigger: Placement location has been identified (For birth or inject)
    // Args:    Organism to be placed, placement position, parent position (if available)
    bool has_BeforePlacement = true;
    virtual void BeforePlacement(Organism &, OrgPosition, OrgPosition) { has_BeforePlacement = false; }    

    // Format:  OnPlacement(OrgPosition placement_pos)
    // Trigger: New organism has been placed in the poulation.
    // Args:    Position new organism was placed.
    bool has_OnPlacement = true;
    virtual void OnPlacement(OrgPosition) { has_OnPlacement = false; }    

    // Format:  BeforeMutate(Organism & org)
    // Trigger: Mutate is about to run on an organism.
    bool has_BeforeMutate = true;
    virtual void BeforeMutate(Organism &) { has_BeforeMutate = false; }    

    // Format:  OnMutate(Organism & org)
    // Trigger: Organism has had its genome changed due to mutation.
    bool has_OnMutate = true;
    virtual void OnMutate(Organism &) { has_OnMutate = false; }    

    // Format:  BeforeDeath(OrgPosition remove_pos)
    // Trigger: Organism is about to die.
    bool has_BeforeDeath = true;
    virtual void BeforeDeath(OrgPosition) { has_BeforeDeath = false; }    

    // Format:  BeforeSwap(OrgPosition pos1, OrgPosition pos2)
    // Trigger: Two organisms' positions in the population are about to move.
    bool has_BeforeSwap = true;
    virtual void BeforeSwap(OrgPosition, OrgPosition) { has_BeforeSwap = false; }    

    // Format:  OnSwap(OrgPosition pos1, OrgPosition pos2)
    // Trigger: Two organisms' positions in the population have just swapped.
    bool has_OnSwap = true;
    virtual void OnSwap(OrgPosition, OrgPosition) { has_OnSwap = false; }    

    // Format:  BeforePopResize(Population & pop, size_t new_size)
    // Trigger: Full population is about to be resized.
    bool has_BeforePopResize = true;
    virtual void BeforePopResize(Population &, size_t) { has_BeforePopResize = false; }    

    // Format:  OnPopResize(Population & pop, size_t old_size)
    // Trigger: Full population has just been resized.
    bool has_OnPopResize = true;
    virtual void OnPopResize(Population &, size_t) { has_OnPopResize = false; }    

    // Format:  OnError(const std::string & msg)
    // Trigger: An error has occurred and the user should be notified.
    bool has_OnError = true;
    virtual void OnError(const std::string &) { has_OnError = false; }

    // Format:  OnWarning(const std::string & msg)
    // Trigger: A atypical condition has occurred and the user should be notified.
    bool has_OnWarning = true;
    virtual void OnWarning(const std::string &) { has_OnWarning = false; }

    // Format:  BeforeExit()
    // Trigger: Run immediately before MABE is about to exit.
    bool has_BeforeExit = true;
    virtual void BeforeExit() { has_BeforeExit = false; }    

    // Format:  OnHelp()
    // Trigger: Run when the --help option is called at startup.
    bool has_OnHelp = true;
    virtual void OnHelp() { has_OnHelp = false; } 


    // Functions to be called based on actions that need to happen.  Each of these returns a
    // viable result or an invalid object if need to pass on to the next module.  Modules will
    // be querried in order until one of them returns a valid result.

    // Function: Place a new organism about to be born.
    // Args: Organism that will be placed, position of parent, position to place.
    // Return: Position to place offspring or an invalid position if failed.

    bool has_DoPlaceBirth = true;
    virtual OrgPosition DoPlaceBirth(Organism &, OrgPosition) {
      has_DoPlaceBirth = false; return OrgPosition();
    }

    // Function: Place a new organism about to be injected.
    // Args: Organism that will be placed, position to place.

    bool has_DoPlaceInject = true;
    virtual OrgPosition DoPlaceInject(Organism &) {
      has_DoPlaceInject = false; return OrgPosition();
    }

    // Function: Find a random neighbor to a designated organism.
    // Args: Position to find neighbor of, position found.

    bool has_DoFindNeighbor = true;
    virtual OrgPosition DoFindNeighbor(OrgPosition) {
      has_DoFindNeighbor = false; return OrgPosition();
    }


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
    virtual void SetupConfig(ConfigScope & config_scope) { ; }

    /// Setup the configuration options for MABE.
    void SetupConfig_Base(ConfigScope & config_scope) {
      auto & module_scope = config_scope.AddScope(name, desc);
      SetupConfig(module_scope);
    }

  };

}

#endif
