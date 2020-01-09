/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  Module.h
 *  @brief Base class for all MABE modules.
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

#include "MABE.h"
#include "ModuleBase.h"
#include "Population.h"
#include "TraitInfo.h"

namespace mabe {

  class Module : public ModuleBase {
  public:
    Module(MABE & in_control, const std::string & in_name, const std::string & in_desc="")
      : ModuleBase(in_control, in_name, in_desc) { ; }
    Module(const Module &) = delete;
    Module(Module &&) = delete;

  protected:

    /// Specialized configuration links for MABE-specific modules.
    /// (Other ways of linking variable to config file are in ConfigType.h)
    ConfigEntry_Functions<std::string> & LinkPop(int & var,
                                                 const std::string & name,
                                                 const std::string & desc,
                                                 int default_pop=0) {
      std::function<std::string()> get_fun =
        [this,&var](){ return control.GetPopulation(var).GetName(); };
      std::function<void(std::string)> set_fun =
        [this,&var](const std::string & name){ var = control.GetPopID(name); };
      return GetScope().LinkFuns<std::string>(
        name, get_fun, set_fun, desc,
        control.GetPopulation(default_pop).GetName());
    }

    // ---== Trait management ==---
   
    /// Add a new trait to this module, specifying its access method, its name, and its description
    /// AND its default value.
    template <typename T>
    TypedTraitInfo<T> & AddTrait(TraitInfo::Access access,
                                 const std::string & name,
                                 const std::string & desc="",
                                 const T & default_val=T()) {
      return control.AddTrait<T>(this, access, name, desc, default_val);
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

  public:

    // Functions to be called based on signals.  Note that the existance of an overridden version
    // of each function is tracked by an associated bool value that we default to true until the
    // base version of the function is called indicating that it has NOT been overridden.

    // Format:  BeforeUpdate(size_t update_ending)
    // Trigger: Update is ending; new one is about to start
    void BeforeUpdate(size_t) override {
      has_signal[SIG_BeforeUpdate] = false;
      control.RescanSignals();
    }

    // Format:  OnUpdate(size_t new_update)
    // Trigger: New update has just started.
    void OnUpdate(size_t) override {
      has_signal[SIG_OnUpdate] = false;
      control.RescanSignals();
    }

    // Format:  BeforeRepro(OrgPosition parent_pos) 
    // Trigger: Parent is about to reproduce.
    void BeforeRepro(OrgPosition) override {
      has_signal[SIG_BeforeRepro] = false;
      control.RescanSignals();
    }

    // Format:  OnOffspringReady(Organism & offspring, OrgPosition parent_pos)
    // Trigger: Offspring is ready to be placed.
    void OnOffspringReady(Organism &, OrgPosition) override {
      has_signal[SIG_OnOffspringReady] = false;
      control.RescanSignals();
    }

    // Format:  OnInjectReady(Organism & inject_org)
    // Trigger: Organism to be injected is ready to be placed.
    void OnInjectReady(Organism &) override {
      has_signal[SIG_OnInjectReady] = false;
      control.RescanSignals();
    }

    // Format:  BeforePlacement(Organism & org, OrgPosition target_pos)
    // Trigger: Placement location has been identified (For birth or inject)
    // Args:    Organism to be placed, placement position, parent position (if available)
    void BeforePlacement(Organism &, OrgPosition, OrgPosition) override {
      has_signal[SIG_BeforePlacement] = false;
      control.RescanSignals();
    }

    // Format:  OnPlacement(OrgPosition placement_pos)
    // Trigger: New organism has been placed in the poulation.
    // Args:    Position new organism was placed.
    void OnPlacement(OrgPosition) override {
      has_signal[SIG_OnPlacement] = false;
      control.RescanSignals();
    }

    // Format:  BeforeMutate(Organism & org)
    // Trigger: Mutate is about to run on an organism.
    void BeforeMutate(Organism &) override {
      has_signal[SIG_BeforeMutate] = false;
      control.RescanSignals();
    }

    // Format:  OnMutate(Organism & org)
    // Trigger: Organism has had its genome changed due to mutation.
    void OnMutate(Organism &) override {
      has_signal[SIG_OnMutate] = false;
      control.RescanSignals();
    }

    // Format:  BeforeDeath(OrgPosition remove_pos)
    // Trigger: Organism is about to die.
    void BeforeDeath(OrgPosition) override {
      has_signal[SIG_BeforeDeath] = false;
      control.RescanSignals();
    }

    // Format:  BeforeSwap(OrgPosition pos1, OrgPosition pos2)
    // Trigger: Two organisms' positions in the population are about to move.
    void BeforeSwap(OrgPosition, OrgPosition) override {
      has_signal[SIG_BeforeSwap] = false;
      control.RescanSignals();
    }

    // Format:  OnSwap(OrgPosition pos1, OrgPosition pos2)
    // Trigger: Two organisms' positions in the population have just swapped.
    void OnSwap(OrgPosition, OrgPosition) override {
      has_signal[SIG_OnSwap] = false;
      control.RescanSignals();
    }

    // Format:  BeforePopResize(Population & pop, size_t new_size)
    // Trigger: Full population is about to be resized.
    void BeforePopResize(Population &, size_t) override {
      has_signal[SIG_BeforePopResize] = false;
      control.RescanSignals();
    }

    // Format:  OnPopResize(Population & pop, size_t old_size)
    // Trigger: Full population has just been resized.
    void OnPopResize(Population &, size_t) override {
      has_signal[SIG_OnPopResize] = false;
      control.RescanSignals();
    }

    // Format:  OnError(const std::string & msg)
    // Trigger: An error has occurred and the user should be notified.
    void OnError(const std::string &) override {
      has_signal[SIG_OnError] = false;
      control.RescanSignals();
    }

    // Format:  OnWarning(const std::string & msg)
    // Trigger: A atypical condition has occurred and the user should be notified.
    void OnWarning(const std::string &) override {
      has_signal[SIG_OnWarning] = false;
      control.RescanSignals();
    }

    // Format:  BeforeExit()
    // Trigger: Run immediately before MABE is about to exit.
    void BeforeExit() override {
      has_signal[SIG_BeforeExit] = false;
      control.RescanSignals();
    }

    // Format:  OnHelp()
    // Trigger: Run when the --help option is called at startup.
    void OnHelp() override {
      has_signal[SIG_OnHelp] = false;
      control.RescanSignals();
    }


    // Functions to be called based on actions that need to happen.  Each of these returns a
    // viable result or an invalid object if need to pass on to the next module.  Modules will
    // be querried in order until one of them returns a valid result.

    // Function: Place a new organism about to be born.
    // Args: Organism that will be placed, position of parent, position to place.
    // Return: Position to place offspring or an invalid position if failed.

    OrgPosition DoPlaceBirth(Organism &, OrgPosition) override {
      has_signal[SIG_DoPlaceBirth] = false;
      control.RescanSignals();
      return OrgPosition();
    }

    // Function: Place a new organism about to be injected.
    // Args: Organism that will be placed, position to place.

    OrgPosition DoPlaceInject(Organism &) override {
      has_signal[SIG_DoPlaceInject] = false;
      control.RescanSignals();
      return OrgPosition();
    }

    // Function: Find a random neighbor to a designated position.
    // Args: Position to find neighbor of, position found.

    OrgPosition DoFindNeighbor(OrgPosition) override {
      has_signal[SIG_DoFindNeighbor] = false;
      control.RescanSignals();
      return OrgPosition();
    }

    /// Turn off all signals in this function.
    void Deactivate() override {
      has_signal.Clear();
      control.RescanSignals();
    }

    /// Turn on all signals in this function (unimplemented ones will turn off automatically)
    void Activate() override {
      has_signal.Clear();
      control.RescanSignals();
    }

  };

  /// Build a class that will automatically register modules when created (globally)
  template <typename T>
  struct ModuleRegistrar {
    ModuleRegistrar(const std::string & type_name, const std::string & desc) {
      ModuleInfo new_info;
      new_info.name = type_name;
      new_info.desc = desc;
      new_info.init_fun = [desc](MABE & control, const std::string & name) -> ConfigType & {
        return control.AddModule<T>(name, desc);
      };
      GetModuleInfo().insert(new_info);
    }
  };

#define MABE_REGISTER_MODULE(TYPE, DESC) \
        mabe::ModuleRegistrar<TYPE> MABE_ ## TYPE ## _Registrar(#TYPE, DESC)

}

#endif
