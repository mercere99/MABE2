/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Module.h
 *  @brief Base class for all MABE modules.
 * 
 *  Development Notes
 *  - Various On* functions should be automatically detected and run when relevant, including:
 *     BeforeRepro(Iterator parent_pos) 
 *       : Parent is about to reporduce.
 *     OnOffspringReady(Organism & offspring, Iterator parent_pos)
 *       : Offspring is ready to be placed.
 *     OnInjectReady(Organism & inject_org)
 *       : Organism to be injected is ready to be placed.
 *     BeforePlacement(Organism & org, Iterator target_pos)
 *       : Placement location has been identified (For birth or inject)
 *     OnPlacement(Iterator placement_pos)
 *       : New organism has been placed in the poulation.
 *     OnMutate(Organism & org)
 *       : Organism has had its genome changed due to mutation.
 *     OnUpdate(size_t ud)
 *       : Regular update is about to occur.
 *     BeforeDeath(Iterator remove_pos)
 *       : Organism is about to die.
 *     OnSwap(Iterator pos1, Iterator pos2)
 *       : Organism's position in the population is about to move.
 *     BeforePopResize(Population & pop, size_t new_size)
 *       : Full population is about to be resized.
 *     OnPopResize(Population & pop, size_t old_size)
 *       : Full population has just been resized.
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

#include "TraitInfo.h"

namespace mabe {

  class MABE;
  class World;

  class Module {
    friend World;
  protected:
    std::string name;                 ///< Unique name for this module.
    std::string desc;                 ///< Description for this module.
    emp::vector<std::string> errors;  ///< Has this class detected any configuration errors?

    // What type of module is this (note, some can be more than one!)
    bool is_evaluate=false;   ///< Does this module perform evaluation on organisms?
    bool is_select=false;     ///< Does this module select organisms to reproduce?
    bool is_placement=false;  ///< Does this module handle offspring placement?
    bool is_mutate=false;     ///< Does this module handle triggering mutations?
    bool is_analyze=false;    ///< Does this module record or evaluate data?

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

    /// Which populations are we operating on?
    size_t min_pops = 0;                           ///< Minimum number of population needed

    /// Which traits is this module working with?
    emp::map<std::string, emp::Ptr<TraitInfo>> trait_map;

    /// Which configuration settings is this module using?
    emp::vector< emp::Ptr<mabe::ConfigLink_Base> > config_links;

    // Helper functions
    template <typename... Ts>
    void AddError(Ts &&... args) {
      errors.push_back( emp::to_string( std::forward<Ts>(args)... ));
      std::cerr << "ERROR: " << errors.back() << std::endl;
    }

  public:
    Module(const std::string & in_name, const std::string & in_desc="")
      : name(in_name), desc(in_desc) { ; }
    Module(const Module &) = default;
    Module(Module &&) = default;
    virtual ~Module() {
      // Clean up trait information.
      for (auto & x : trait_map) x.second.Delete();
      for (auto ptr : config_links) ptr.Delete();
    }

    const std::string & GetName() const noexcept { return name; }
    bool HasErrors() const { return errors.size(); }
    const emp::vector<std::string> & GetErrors() const noexcept { return errors; }
    size_t GetMinPops() const noexcept { return min_pops; }

    virtual emp::Ptr<Module> Clone() { return nullptr; }

    bool IsEvaluate() const noexcept  { return is_evaluate; }
    bool IsSelect() const noexcept  { return is_select; }
    bool IsPlacement() const noexcept  { return is_placement; }
    bool IsMutate() const noexcept  { return is_mutate; }
    bool IsAnalyze() const noexcept  { return is_analyze; }

    Module & IsEvaluate(bool in) noexcept { is_evaluate = in; return *this; }
    Module & IsSelect(bool in) noexcept { is_select = in; return *this; }
    Module & IsPlacement(bool in) noexcept { is_placement = in; return *this; }
    Module & IsMutate(bool in) noexcept { is_mutate = in; return *this; }
    Module & IsAnalyze(bool in) noexcept { is_analyze = in; return *this; }

    Module & RequireAsync() { rep_type = ReplicationType::REQUIRE_ASYNC; return *this; }
    Module & DefaultAsync() { rep_type = ReplicationType::DEFAULT_ASYNC; return *this; }
    Module & DefaultSync() { rep_type = ReplicationType::DEFAULT_SYNC; return *this; }
    Module & RequireSync() { rep_type = ReplicationType::REQUIRE_SYNC; return *this; }

    virtual void Setup(mabe::World &) { /* By default, assume no setup needed. */ }
    virtual void Update(mabe::World &) { /* By default, do nothing at update. */ }

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

    /// Link a module variable to a configuration setting.
    template <typename T>
    Module & LinkConfigVar(T & var, const std::string & name, const std::string & desc="") {
      config_links.push_back( emp::NewPtr<ConfigLink<T>>(var, name, desc) );
      return *this;
    }

    Module & OutputConfigSettings(std::ostream & os=std::cout, const std::string & prefix="") {
      os << prefix << "# " << desc << "\n"
         << prefix << name << " = {\n";

      // Print each variable for this module.
      for (size_t i = 0; i < config_links.size(); i++) {
        if (i) os << "\n";          // Skip lines internal to the module.
        config_links[i]->Write(os, prefix+"  ");  // Print out each config link.
      }

      os << prefix << "}" << std::endl;

      return *this;
    }

  };

}

#endif
