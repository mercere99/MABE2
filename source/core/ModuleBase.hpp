/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ModuleBase.hpp
 *  @brief Base class for Module, which (in turn) is the base class for all MABE modules
 * 
 *  Core module functionality is split between ModuleBase (this class) and Module (which is
 *  derived from this class).  The difference is that the main MABE controller has access only
 *  to ModuleBase.  Module, in turn, has access to the main MABE controller.  So:
 * 
 *     ModuleBase  <-  MABE  <-  Module
 *
 *  When you are developing a new class derived from Module, you will be able to access the
 *  MABE controller and make any changes to it that you need to.  The MABE controller will have
 *  access to base Module functionality through this class, ModuleBase.
 * 
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
 *     OnOffspringReady(Organism & offspring, OrgPosition parent_pos, Population & target_pop)
 *       : Offspring is ready to be placed.
 *     OnInjectReady(Organism & inject_org, Population & pop)
 *       : Organism to be injected into pop is ready to be placed.
 *     BeforePlacement(Organism & org, OrgPosition target_pos, OrgPosition parent_pos)
 *       : Placement location has been identified (For birth or inject)
 *     OnPlacement(OrgPosition placement_pos)
 *       : New organism has been placed in the population.
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
 *     BeforeExit()
 *       : Run immediately before MABE is about to exit.
 *     OnHelp()
 *       : Run when the --help option is called at startup.
 *     ...
 */

#ifndef MABE_MODULE_BASE_H
#define MABE_MODULE_BASE_H

#include <set>
#include <string>

#include "emp/base/map.hpp"
#include "emp/base/notify.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/datastructs/reference_vector.hpp"

#include "../Emplode/Emplode.hpp"

#include "TraitInfo.hpp"

namespace mabe {

  class MABE;
  class OrgType;
  class Organism;
  class OrgPosition;
  class Population;

  using emplode::EmplodeType;

  class ModuleBase : public EmplodeType {
    friend MABE;
  protected:
    std::string name;          ///< Unique name for this module.
    std::string desc;          ///< Description for this module.
    mabe::MABE & control;      ///< Reference to main mabe controller using module
    bool is_builtin=false;     ///< Is this a built-in module not for config?

    /// Informative tags about this module.  Expected tags include:
    ///   "Analyze"     : Makes measurements on the population.
    ///   "Archive"     : Store specific types of data.
    ///   "Evaluate"    : Examines organisms and annotates the data map.
    ///   "Interface"   : Provides mechanisms for the user to interact with the world.
    ///   "ManageOrgs"  : Manages a type of organism in the world.
    ///   "Mutate"      : Modifies organism genomes
    ///   "Placement"   : Identifies where new organisms should be placed in the population.
    ///   "Select"      : Chooses organisms to act as parents in for the next generation.
    ///   "Visualize"   : Displays data for the user.
    std::set<std::string> action_tags; ///< Informative tags about this model

    /// Set of traits that this module is working with.
    emp::map<std::string, emp::Ptr<TraitInfo>> trait_map;

    /// Other variables that we want to hook on to this Module externally.
    emp::DataMap data_map;

    using value_fun_t = std::function<double(emp::DataMap &)>;
    using string_fun_t = std::function<std::string(emp::DataMap &)>;

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
      SIG_BeforeExit,
      SIG_OnHelp,
      NUM_SIGNALS,
      SIG_UNKNOWN
    };

  protected:
    // Setup a BitSet to track if this module has each signal implemented.
    emp::BitSet<NUM_SIGNALS> has_signal;

    // Core implementation for ManagerModule functionality.
    virtual emp::Ptr<OrgType> CloneObject_impl(const OrgType &) {
      emp_assert(false, "CloneObject_impl() must be overridden for ManagerModule.");
      return nullptr;
    }
    virtual emp::Ptr<OrgType> CloneObject_impl(const OrgType &, emp::Random &) {
      emp_assert(false, "CloneObject_impl() must be overridden for ManagerModule.");
      return nullptr;
    }
    virtual emp::Ptr<OrgType> Make_impl() {
      emp_assert(false, "Make_impl() must be overridden for ManagerModule.");
      return nullptr;
    }
    virtual emp::Ptr<OrgType> Make_impl(emp::Random &) {
      emp_assert(false, "Make_impl() must be overridden for ManagerModule.");
      return nullptr;
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

    /// By DEFAULT modules do not do anything extra when copying themselves.
    bool CopyValue(const EmplodeType &) override { return true; }

    /// By DEFAULT modules do not do anything to setup configurations.
    void SetupConfig() override { }

    const std::string & GetName() const noexcept { return name; }
    const std::string & GetDesc() const noexcept { return desc; }

    virtual std::string GetTypeName() const { return "ModuleBase"; }
    virtual emp::Ptr<ModuleBase> Clone() { return nullptr; }

    bool IsBuiltIn() const { return is_builtin; }
    void SetBuiltIn(bool _in=true) { is_builtin = _in; }

    bool IsAnalyzeMod() const { return emp::Has(action_tags, "Analyze"); }
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
    ModuleBase & SetEvaluateMod(bool in=true) { return SetActionTag("Evaluate", in); }
    ModuleBase & SetInterfaceMod(bool in=true) { return SetActionTag("Interface", in); }
    ModuleBase & SetManageMod(bool in=true) { return SetActionTag("ManageOrgs", in); }
    ModuleBase & SetMutateMod(bool in=true) { return SetActionTag("Mutate", in); }
    ModuleBase & SetPlacementMod(bool in=true) { return SetActionTag("Placement", in); }
    ModuleBase & SetSelectMod(bool in=true) { return SetActionTag("Select", in); }
    ModuleBase & SetVisualizerMod(bool in=true) { return SetActionTag("Visualize", in); }

    // Allow modules to setup any traits or other internal state after config is loaded.
    virtual void SetupModule() { /* By default, assume no setup needed. */ }

    // Once data maps are locked in (no new traits allowed) modules can use that information.
    virtual void SetupDataMap(emp::DataMap &) { /* By default, no setup needed. */ }

    // ----==== SIGNALS ====----

    // Base classes for signals to be called (More details in Module.h)

    virtual void BeforeUpdate(size_t) = 0;
    virtual void OnUpdate(size_t) = 0;
    virtual void BeforeRepro(OrgPosition) = 0;
    virtual void OnOffspringReady(Organism &, OrgPosition, Population &) = 0;
    virtual void OnInjectReady(Organism &, Population &) = 0;
    virtual void BeforePlacement(Organism &, OrgPosition, OrgPosition) = 0;
    virtual void OnPlacement(OrgPosition) = 0;
    virtual void BeforeMutate(Organism &) = 0;
    virtual void OnMutate(Organism &) = 0;
    virtual void BeforeDeath(OrgPosition) = 0;
    virtual void BeforeSwap(OrgPosition, OrgPosition) = 0;
    virtual void OnSwap(OrgPosition, OrgPosition) = 0;
    virtual void BeforePopResize(Population &, size_t) = 0;
    virtual void OnPopResize(Population &, size_t) = 0;
    virtual void BeforeExit() = 0;
    virtual void OnHelp() = 0;

    virtual void Deactivate() = 0;  ///< Turn off all signals in this function.
    virtual void Activate() = 0;    ///< Turn on all signals in this function.

    virtual bool BeforeUpdate_IsTriggered() = 0;
    virtual bool OnUpdate_IsTriggered() = 0;
    virtual bool BeforeRepro_IsTriggered() = 0;
    virtual bool OnOffspringReady_IsTriggered() = 0;
    virtual bool OnInjectReady_IsTriggered() = 0;
    virtual bool BeforePlacement_IsTriggered() = 0;
    virtual bool OnPlacement_IsTriggered() = 0;
    virtual bool BeforeMutate_IsTriggered() = 0;
    virtual bool OnMutate_IsTriggered() = 0;
    virtual bool BeforeDeath_IsTriggered() = 0;
    virtual bool BeforeSwap_IsTriggered() = 0;
    virtual bool OnSwap_IsTriggered() = 0;
    virtual bool BeforePopResize_IsTriggered() = 0;
    virtual bool OnPopResize_IsTriggered() = 0;
    virtual bool BeforeExit_IsTriggered() = 0;
    virtual bool OnHelp_IsTriggered() = 0;

    virtual bool OK() const = 0;  // For debugging purposes only.

    // ---=== Specialty Functions for Organism Managers ===---
    virtual emp::TypeID GetObjType() const {
      emp_assert(false, "GetObjType() must be overridden for ManagerModule.");
      return emp::TypeID();
    }
    template <typename OBJ_T>
    emp::Ptr<OBJ_T> CloneObject(const OBJ_T & in_obj) {
      return CloneObject_impl(in_obj).template DynamicCast<OBJ_T>();
    }
    template <typename OBJ_T>
    emp::Ptr<OBJ_T> CloneObject(const OBJ_T & in_obj, emp::Random & random) {
      return CloneObject_impl(in_obj, random).template DynamicCast<OBJ_T>();
    }
    template <typename OBJ_T>
    emp::Ptr<OBJ_T> Make() {
      return Make_impl().template DynamicCast<OBJ_T>();
    }
    template <typename OBJ_T>
    emp::Ptr<OBJ_T> Make(emp::Random & random) {
      return Make_impl(random).template DynamicCast<OBJ_T>();
    }
  };

  struct ModuleInfo {
    std::string name;
    std::string desc;
    std::function<emp::Ptr<EmplodeType>(MABE &, const std::string &)> obj_init_fun;
    std::function<void(emplode::TypeInfo &)> type_init_fun;
    emp::TypeID type_id;
    bool operator<(const ModuleInfo & in) const { return name < in.name; }
  };

  static std::map<std::string,ModuleInfo> & GetModuleMap() {
    static std::map<std::string,ModuleInfo> mod_type_map;
    return mod_type_map;
  }

  static void PrintModuleInfo() {
    auto & mod_type_map = GetModuleMap();
    for (auto & [name,mod] : mod_type_map) {
      std::cout << name << " : " << mod.desc << std::endl;
    }
  }
}

#endif
