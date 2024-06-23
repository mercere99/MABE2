/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2024.
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

#include "emp/base/assert.hpp"
#include "emp/base/map.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/datastructs/reference_vector.hpp"
#include "emp/tools/String.hpp"

#include "MABE.hpp"
#include "ModuleBase.hpp"
#include "OrgTrait.hpp"
#include "Population.hpp"
#include "TraitInfo.hpp"

namespace mabe {

  class Module : public ModuleBase {
  public:
    Module(MABE & in_control, const emp::String & in_name, const emp::String & in_desc="")
      : ModuleBase(in_control, in_name, in_desc) { }
    Module(const Module &) = delete;
    Module(Module &&) = delete;

    using Access = TraitInfo::Access;

    struct UpdateRange {
      size_t start = 0;
      size_t stop = emp::MAX_SIZE_T;
      size_t step = 1;

      bool IsValid(size_t in_update) {
        return in_update >= start &&
               in_update <= stop &&
               (in_update - start) % step == 0;
      }
    };

  protected:

    EMP_DEBUG(
      bool setup_config_internal_run = false;
      bool setup_module_internal_run = false;
      bool setup_data_map_internal_run = false;
    )

    TraitManager<ModuleBase> & GetTraitManager() override { return control.GetTraitManager(); }

    void SetupConfig_Internal() override {
      emp_assert( setup_config_internal_run == false,
                  "SetupConfig_Internal() should be run only once.");
      EMP_DEBUG( setup_config_internal_run = true; )

      // Link all module traits to an associated config variable that specifies trait name.
      for (auto trait_ptr : trait_ptrs) {
        LinkVar(trait_ptr->GetNameVar(),
                trait_ptr->GetConfigName(),
                trait_ptr->GetConfigDesc());
      }      
    }

    void SetupModule_Internal() override final {
      emp_assert( setup_module_internal_run == false,
                  "SetupModule_Internal() should be run only once.");
      EMP_DEBUG( setup_module_internal_run = true; )

      // Now that we know all trait names, make sure those names are registered correctly.
      for (auto trait_ptr : trait_ptrs) {
        trait_ptr->AddTrait();
      }
    }

    void SetupDataMap_Internal(emp::DataMap & dm) override final {
      emp_assert( setup_data_map_internal_run == false,
                  "SetupDataMap_Internal() should be run only once.");
      EMP_DEBUG( setup_data_map_internal_run = true; )

      // Now that all traits are registered, make sure they know their position in the DataMap.
      for (auto trait_ptr : trait_ptrs) {
        trait_ptr->SetupDataMap(dm);
      }      
    }

    // Specialized configuration links for MABE-specific modules.
    // (Other ways of linking variable to config file are in EmplodeType.h)

    /// Link a single population to a parameter by name.
    emplode::Symbol_LinkedFunctions<emp::String> & LinkPop(
      int & var,
      const emp::String & name,
      const emp::String & desc
    ) {
      std::function<emp::String()> get_fun =
        [this,&var](){ return control.GetPopulation(var).GetName(); };

      std::function<void(emp::String)> set_fun =
        [this,&var](const emp::String & name){
          var = control.GetPopID(name);
          if (var == -1) {
            emp::notify::Error("Trying to access population '", name, "'; does not exist.");
          }
        };

      return AsScope().LinkFuns<emp::String>(name, get_fun, set_fun, desc);
    }

    /// Link one or more populations (or portions of a population) to a parameter.
    emplode::Symbol_LinkedFunctions<emp::String> & LinkCollection(
      mabe::Collection & var,
      const emp::String & name,
      const emp::String & desc
    ) {
      std::function<emp::String()> get_fun =
        [this,&var](){ return control.ToString(var); };

      std::function<void(emp::String)> set_fun =
        [this,&var](const emp::String & load_str){
          var = control.ToCollection(load_str);
        };

      return AsScope().LinkFuns<emp::String>(name, get_fun, set_fun, desc);
    }

    /// Link another module to this one, by name (track using int ID)
    emplode::Symbol_LinkedFunctions<emp::String> & LinkModule(
      int & var,
      const emp::String & name,
      const emp::String & desc
    ) {
      std::function<emp::String()> get_fun =
        [this,&var](){ return control.GetModule(var).GetName(); };

      std::function<void(emp::String)> set_fun =
        [this,&var](const emp::String & name){
          var = control.GetModuleID(name);
          if (var == -1) emp::notify::Error("Trying to access module '", name, "'; does not exist.");
        };

      return AsScope().LinkFuns<emp::String>(name, get_fun, set_fun, desc);
    }

    /// Link a range of values with a start, stop, and step.
    template <typename T=int>
    emplode::Symbol_LinkedFunctions<emp::String> & LinkRange(
      T & start_var,
      T & step_var,
      T & stop_var,
      const emp::String & name,
      const emp::String & desc
    ) {
      constexpr T no_val = ((T) 0) - 1;
      std::function<emp::String()> get_fun =
        [&start_var,&step_var,&stop_var]() {
          // If stop_var is no_val, don't bother printing it (i.e. NO stop)
          if (stop_var == no_val) return emp::MakeString(start_var, ':', step_var);
          return emp::MakeString(start_var, ':', step_var, ':', stop_var);
        };

      std::function<void(emp::String)> set_fun =
        [&start_var,&step_var,&stop_var](emp::String name){
          start_var = emp::from_string<T>(emp::string_pop(name, ':'));
          step_var = emp::from_string<T>(emp::string_pop(name, ':'));
          stop_var = name.size() ? emp::from_string<T>(name) : no_val; // no_val indicates no stop.
        };

      return AsScope().LinkFuns<emp::String>(name, get_fun, set_fun, desc);
    }

    auto LinkRange(UpdateRange & in_range, const emp::String & name, const emp::String & desc) {
      return LinkRange(in_range.start, in_range.step, in_range.stop, name, desc);
    }
  public:

    /// If we don't have a MABE controller for a test, do any needed setup.
    void SetupTestMode() {
      SetupModule_Internal();
      SetupModule();
    }

    /// Make sure all setup commands have been run.
    bool IsSetup() const {
      if constexpr (!emp::is_debug_mode) return true;
      EMP_DEBUG(
        return setup_config_internal_run &&
              setup_data_map_internal_run &&
              setup_module_internal_run;
      )
    }

    // ---== Trait management ==---
   
    /// Add a new trait to this module, specifying its access method, its name, and its description
    /// AND its default value.
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddTrait(Access access,
                         const emp::String & name,
                         const emp::String & desc="",
                         const T & default_val=T(),
                         size_t count=1
    ) {
      emp_assert(name != "", name);
      return control.GetTraitManager().AddTrait<T,ALT_Ts...>(this, access, name, desc, default_val, count);
    }

    /// Add trait that this module can READ & WRITE this trait.  Others cannot use it.
    /// Must provide name, description, and a default value to start at.
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddPrivateTrait(const emp::String & name, const emp::String & desc, const T & default_val, size_t count=1) {
      return AddTrait<T,ALT_Ts...>(Access::PRIVATE, name, desc, default_val, count);
    }

    /// Add trait that this module can READ & WRITE to; other modules can only read.
    /// Must provide name, description, and a default value to start at.
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddOwnedTrait(const emp::String & name, const emp::String & desc, const T & default_val, size_t count=1) {
      return AddTrait<T,ALT_Ts...>(Access::OWNED, name, desc, default_val, count);
    }
   
    /// Add trait that this module can READ & WRITE to; at least one other module MUST read it.
    /// Must provide name, description, and a default value to start at.
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddGeneratedTrait(const emp::String & name, const emp::String & desc, const T & default_val, size_t count=1) {
      return AddTrait<T,ALT_Ts...>(Access::GENERATED, name, desc, default_val, count);
    }
   
    /// Add trait that this module can READ & WRITE; other modules can too.
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddSharedTrait(const emp::String & name, const emp::String & desc, const T & default_val, size_t count=1) {
      return AddTrait<T,ALT_Ts...>(Access::SHARED, name, desc, default_val, count);
    }
   
    /// Add trait that this module will use if it exists, but only if another module is also
    /// going to use it.
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddOptionalTrait(const emp::String & name, const emp::String & desc, size_t count=1) {
      return AddTrait<T,ALT_Ts...>(Access::OPTIONAL, name, desc, T(), count);
    }

    /// Add trait that this module can READ, but another module must WRITE to it.
    /// That other module should also provide the description for the trait.
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddRequiredTrait(const emp::String & name, size_t count=1) {
      return AddTrait<T,ALT_Ts...>(Access::REQUIRED, name, "", T(), count);
    }
    
    /// Add trait that this module can READ, but is not required 
    template <typename T, typename... ALT_Ts>
    TraitInfo & AddOptionalTrait(const emp::String & name) {
      return AddTrait<T,ALT_Ts...>(TraitInfo::Access::OPTIONAL, name);
    }

    /// Add all of the traits that that this module needs to be able to READ, in order to
    /// computer the provided equation.  Another module must WRITE these traits and provide the
    /// descriptions.
    void AddRequiredEquation(const emp::String & equation) {
      const std::set<emp::String> & traits = control.GetEquationTraits(equation);
      for (const emp::String & name : traits) AddRequiredTrait<double,int,size_t>(name);
    }


    // ---== Signal Handling ==---

    // Functions to be called based on signals.  Note that the existence of an overridden version
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
    // Trigger: New organism has been placed in the population.
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
    bool BeforeExit_IsTriggered() override { return control.BeforeExit_IsTriggered(this); };
    bool OnHelp_IsTriggered() override { return control.OnHelp_IsTriggered(this); };

    bool OK() const override { return true; }
  };

  /// Build a class that will automatically register modules when created (globally)
  template <typename T>
  struct ModuleRegistrar {
    ModuleRegistrar(const emp::String & type_name, const emp::String & desc) {
      emp_assert(!emp::Has(GetModuleMap(), type_name), "Module name used multiple times.", type_name);
      ModuleInfo new_info;
      new_info.name = type_name;
      new_info.full_desc = desc.Slice(desc, '\n');
      new_info.brief_desc = new_info.full_desc.size() ? new_info.full_desc[0] : "(no description available)";
      new_info.obj_init_fun = [desc](MABE & control, const emp::String & name) -> emp::Ptr<EmplodeType> {
        return &control.AddModule<T>(name, desc);
      };
      new_info.type_init_fun = [](emplode::TypeInfo & info){ T::InitType(info); };
      new_info.type_id = emp::GetTypeID<T>();
      GetModuleMap()[type_name] = new_info;
    }
  };

#define MABE_REGISTER_MODULE(TYPE, DESC) \
        mabe::ModuleRegistrar<TYPE> MABE_ ## TYPE ## _Registrar(#TYPE, DESC)

}

#endif
