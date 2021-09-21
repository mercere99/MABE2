/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  Module.hpp
 *  @brief Base class for all MABE modules.
 * 
 *  Core module functionality is split between ModuleBase (which this class is derived from) and
 *  Module (this class).  The difference is that the main MABE controller has access only
 *  to ModuleBase.  Module, in turn, has access to the main MABE controller.  So:
 * 
 *     ModuleBase  <-  MABE  <-  Module
 *
 *  When you are developing a new class derived from Module, you will be able to access the
 *  MABE controller and make any changes to it that you need to.  The MABE controller will have
 *  access to base Module functionality through this ModuleBase.
 * 
 */

#ifndef MABE_MODULE_H
#define MABE_MODULE_H

#include <string>

#include "emp/base/map.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/datastructs/reference_vector.hpp"

#include "../config/Config.hpp"

#include "MABE.hpp"
#include "ModuleBase.hpp"
#include "Population.hpp"
#include "TraitInfo.hpp"

namespace mabe {

  class Module : public ModuleBase {
  public:
    Module(MABE & in_control, const std::string & in_name, const std::string & in_desc="")
      : ModuleBase(in_control, in_name, in_desc) { error_man = &control.GetErrorManager(); }
    Module(const Module &) = delete;
    Module(Module &&) = delete;

  protected:

    // Specialized configuration links for MABE-specific modules.
    // (Other ways of linking variable to config file are in ConfigType.h)

    /// Link a single population to a parameter by name.
    ConfigEntry_Functions<std::string> & LinkPop(int & var,
                                                 const std::string & name,
                                                 const std::string & desc) {
      std::function<std::string()> get_fun =
        [this,&var](){ return control.GetPopulation(var).GetName(); };

      std::function<void(std::string)> set_fun =
        [this,&var](const std::string & name){
          var = control.GetPopID(name);
          if (var == -1) AddError("Trying to access population '", name, "'; does not exist.");
        };

      return GetScope().LinkFuns<std::string>(name, get_fun, set_fun, desc);
    }

    /// Link one or more populations (or portions of a population) to a parameter.
    ConfigEntry_Functions<std::string> & LinkCollection(mabe::Collection & var,
                                                        const std::string & name,
                                                        const std::string & desc) {
      std::function<std::string()> get_fun =
        [this,&var](){ return control.ToString(var); };

      std::function<void(std::string)> set_fun =
        [this,&var](const std::string & load_str){
          var = control.FromString(load_str);
        };

      return GetScope().LinkFuns<std::string>(name, get_fun, set_fun, desc);
    }

    /// Link another module to this one, by name (track using int ID)
    ConfigEntry_Functions<std::string> & LinkModule(int & var,
                                                    const std::string & name,
                                                    const std::string & desc) {
      std::function<std::string()> get_fun =
        [this,&var](){ return control.GetModule(var).GetName(); };

      std::function<void(std::string)> set_fun =
        [this,&var](const std::string & name){
          var = control.GetModuleID(name);
          if (var == -1) AddError("Trying to access module '", name, "'; does not exist.");
        };

      return GetScope().LinkFuns<std::string>(name, get_fun, set_fun, desc);
    }

    /// Link a range of values with a start, stop, and step.
    ConfigEntry_Functions<std::string> & LinkRange(int & start_var,
                                                   int & step_var,
                                                   int & stop_var,
                                                   const std::string & name,
                                                   const std::string & desc) {
      std::function<std::string()> get_fun =
        [&start_var,&step_var,&stop_var]() {
          // If stop_var is -1, don't bother printing it (i.e. NO stop)
          if (stop_var == -1) return emp::to_string(start_var, ':', step_var);
          return emp::to_string(start_var, ':', step_var, ':', stop_var);
        };

      std::function<void(std::string)> set_fun =
        [&start_var,&step_var,&stop_var](std::string name){
          start_var = emp::from_string<int>(emp::string_pop(name, ':'));
          step_var = emp::from_string<int>(emp::string_pop(name, ':'));
          stop_var = name.size() ? emp::from_string<int>(name) : -1; // -1 indicates no stop.
        };

      return GetScope().LinkFuns<std::string>(name, get_fun, set_fun, desc);
    }
  public:

    // ---== Trait management ==---
   
    /// Add a new trait to this module, specifying its access method, its name, and its description
    /// AND its default value.
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddTrait(TraitInfo::Access access,
                         const std::string & name,
                         const std::string & desc="",
                         const T & default_val=T()) {
      return control.GetTraitManager().AddTrait<T,ALT_Ts...>(this, access, name, desc, default_val);
    }

    /// Add trait that this module can READ & WRITE this trait.  Others cannot use it.
    /// Must provide name, description, and a default value to start at.
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddPrivateTrait(const std::string & name, const std::string & desc, const T & default_val) {
      return AddTrait<T,ALT_Ts...>(TraitInfo::Access::PRIVATE, name, desc, default_val);
    }

    /// Add trait that this module can READ & WRITE to; other modules can only read.
    /// Must provide name, description, and a default value to start at.
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddOwnedTrait(const std::string & name, const std::string & desc, const T & default_val) {
      return AddTrait<T,ALT_Ts...>(TraitInfo::Access::OWNED, name, desc, default_val);
    }
   
    /// Add trait that this module can READ & WRITE to; at least one other module MUST read it.
    /// Must provide name, description, and a default value to start at.
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddGeneratedTrait(const std::string & name, const std::string & desc, const T & default_val) {
      return AddTrait<T,ALT_Ts...>(TraitInfo::Access::GENERATED, name, desc, default_val);
    }
   
    /// Add trait that this module can READ & WRITE; other modules can too.
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddSharedTrait(const std::string & name, const std::string & desc, const T & default_val) {
      return AddTrait<T,ALT_Ts...>(TraitInfo::Access::SHARED, name, desc, default_val);
    }
   
    /// Add trait that this module can READ, but another module must WRITE to it.
    /// That other module should also provide the description for the trait.
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddRequiredTrait(const std::string & name) {
      return AddTrait<T,ALT_Ts...>(TraitInfo::Access::REQUIRED, name);
    }


    // ---== Signal Handling ==---

    // Functions to be called based on signals.  Note that the existance of an overridden version
    // of each function is tracked by an associated bool value that we default to true until the
    // base version of the function is called indicating that it has NOT been overridden.

    // Format:  BeforeUpdate(size_t update_ending)
    // Trigger: Update is ending; new one is about to start
    // Args:    Update ID that is just finishing.
    void BeforeUpdate(size_t) override {
      has_signal[SIG_BeforeUpdate] = false;
      control.RescanSignals();
    }

    // Format:  OnUpdate(size_t new_update)
    // Trigger: New update has just started.
    // Args:    Update ID just starting.
    void OnUpdate(size_t) override {
      has_signal[SIG_OnUpdate] = false;
      control.RescanSignals();
    }

    // Format:  BeforeRepro(OrgPosition parent_pos) 
    // Trigger: Parent is about to reproduce.
    // Args:    Position of organism about to reproduce.
    void BeforeRepro(OrgPosition) override {
      has_signal[SIG_BeforeRepro] = false;
      control.RescanSignals();
    }

    // Format:  OnOffspringReady(Organism & offspring, OrgPosition parent_pos, Population & target_pop)
    // Trigger: Offspring is ready to be placed.
    // Args:    Offspring to be born, position of parent, population to place offspring in.
    void OnOffspringReady(Organism &, OrgPosition, Population &) override {
      has_signal[SIG_OnOffspringReady] = false;
      control.RescanSignals();
    }

    // Format:  OnInjectReady(Organism & inject_org, Population & target_pop)
    // Trigger: Organism to be injected is ready to be placed.
    // Args:    Organism to be injected, population to inject into.
    void OnInjectReady(Organism &, Population &) override {
      has_signal[SIG_OnInjectReady] = false;
      control.RescanSignals();
    }

    // Format:  BeforePlacement(Organism & org, OrgPosition target_pos, OrgPosition parent_pos)
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
    // Args:    Organism about to mutate.
    void BeforeMutate(Organism &) override {
      has_signal[SIG_BeforeMutate] = false;
      control.RescanSignals();
    }

    // Format:  OnMutate(Organism & org)
    // Trigger: Organism has had its genome changed due to mutation.
    // Args:    Organism that just mutated.
    void OnMutate(Organism &) override {
      has_signal[SIG_OnMutate] = false;
      control.RescanSignals();
    }

    // Format:  BeforeDeath(OrgPosition remove_pos)
    // Trigger: Organism is about to die.
    // Args:    Position of organism about to die.
    void BeforeDeath(OrgPosition) override {
      has_signal[SIG_BeforeDeath] = false;
      control.RescanSignals();
    }

    // Format:  BeforeSwap(OrgPosition pos1, OrgPosition pos2)
    // Trigger: Two organisms' positions in the population are about to move.
    // Args:    Positions of organisms about to be swapped.
    void BeforeSwap(OrgPosition, OrgPosition) override {
      has_signal[SIG_BeforeSwap] = false;
      control.RescanSignals();
    }

    // Format:  OnSwap(OrgPosition pos1, OrgPosition pos2)
    // Trigger: Two organisms' positions in the population have just swapped.
    // Args:    Positions of organisms just swapped.
    void OnSwap(OrgPosition, OrgPosition) override {
      has_signal[SIG_OnSwap] = false;
      control.RescanSignals();
    }

    // Format:  BeforePopResize(Population & pop, size_t new_size)
    // Trigger: Full population is about to be resized.
    // Args:    Population about to be resized, the size it will become.
    void BeforePopResize(Population &, size_t) override {
      has_signal[SIG_BeforePopResize] = false;
      control.RescanSignals();
    }

    // Format:  OnPopResize(Population & pop, size_t old_size)
    // Trigger: Full population has just been resized.
    // Args:    Population just resized, previous size it was.
    void OnPopResize(Population &, size_t) override {
      has_signal[SIG_OnPopResize] = false;
      control.RescanSignals();
    }

    // Format:  OnError(const std::string & msg)
    // Trigger: An error has occurred and the user should be notified.
    // Args:    Message associated with this error.
    void OnError(const std::string &) override {
      has_signal[SIG_OnError] = false;
      control.RescanSignals();
    }

    // Format:  OnWarning(const std::string & msg)
    // Trigger: A atypical condition has occurred and the user should be notified.
    // Args:    Message associated with this warning.
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

    // Format:  TraceEval(Organism & trace_org, std::ostream & out_stream)
    // Trigger: Request to print a trace of the evaluation of an organism.
    // Args:    Organism to be traces, stream to print trace to.
    void TraceEval(Organism &, std::ostream &) override {
      has_signal[SIG_TraceEval] = false;
      control.RescanSignals();
    }


    // Functions to be called based on actions that need to happen.  Each of these returns a
    // viable result or an invalid object if need to pass on to the next module.  Modules will
    // be querried in order until one of them returns a valid result.

    // Function: Place a new organism about to be born.
    // Args: Organism that will be placed, position of parent, population to place.
    // Return: Position to place offspring or an invalid position if failed.

    OrgPosition DoPlaceBirth(Organism &, OrgPosition, Population &) override {
      has_signal[SIG_DoPlaceBirth] = false;
      control.RescanSignals();
      return OrgPosition();
    }

    // Function: Place a new organism about to be injected.
    // Args: Organism that will be placed, position to place.

    OrgPosition DoPlaceInject(Organism &, Population &) override {
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
      has_signal.SetAll();
      control.RescanSignals();
    }

    bool BeforeUpdate_IsTriggered() override { return control.BeforeUpdate_IsTriggered(this); };
    bool OnUpdate_IsTriggered() override { return control.OnUpdate_IsTriggered(this); };
    bool BeforeRepro_IsTriggered() override { return control.BeforeRepro_IsTriggered(this); };
    bool OnOffspringReady_IsTriggered() override { return control.OnOffspringReady_IsTriggered(this); };
    bool OnInjectReady_IsTriggered() override { return control.OnInjectReady_IsTriggered(this); };
    bool BeforePlacement_IsTriggered() override { return control.BeforePlacement_IsTriggered(this); };
    bool OnPlacement_IsTriggered() override { return control.OnPlacement_IsTriggered(this); };
    bool BeforeMutate_IsTriggered() override { return control.BeforeMutate_IsTriggered(this); };
    bool OnMutate_IsTriggered() override { return control.OnMutate_IsTriggered(this); };
    bool BeforeDeath_IsTriggered() override { return control.BeforeDeath_IsTriggered(this); };
    bool BeforeSwap_IsTriggered() override { return control.BeforeSwap_IsTriggered(this); };
    bool OnSwap_IsTriggered() override { return control.OnSwap_IsTriggered(this); };
    bool BeforePopResize_IsTriggered() override { return control.BeforePopResize_IsTriggered(this); };
    bool OnPopResize_IsTriggered() override { return control.OnPopResize_IsTriggered(this); };
    bool OnError_IsTriggered() override { return control.OnError_IsTriggered(this); };
    bool OnWarning_IsTriggered() override { return control.OnWarning_IsTriggered(this); };
    bool BeforeExit_IsTriggered() override { return control.BeforeExit_IsTriggered(this); };
    bool OnHelp_IsTriggered() override { return control.OnHelp_IsTriggered(this); };
    bool TraceEval_IsTriggered() override { return control.TraceEval_IsTriggered(this); };
    bool DoPlaceBirth_IsTriggered() override { return control.DoPlaceBirth_IsTriggered(this); };
    bool DoPlaceInject_IsTriggered() override { return control.DoPlaceInject_IsTriggered(this); };
    bool DoFindNeighbor_IsTriggered() override { return control.DoFindNeighbor_IsTriggered(this); };
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
