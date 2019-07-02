/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
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

namespace mabe {

  class MABE;
  class World;

  class Module {
  protected:
    std::string name;                 ///< Unique name for this module.
    emp::vector<std::string> errors;  ///< Has this class detected any configuration errors?

    // What type of module is this (note, some can be more than one!)
    bool is_evaluate=false;   ///< Does this module perform evaluation on organisms?
    bool is_select=false;     ///< Does this module select organisms to reproduce?
    bool is_placement=false;  ///< Does this module handle offspring placement?
    bool is_analyze=false;    ///< Does this module record or evaluate data?

    /// Is this module expecting sychronous replication (i.e., discrete generations) or
    /// asynchronous replication (i.e., overlapping generations)?  The former is more common
    /// in evolutionary computation, while the latter is more common in artificial life.
    /// Modules with no-preference will be ignored.  Modules with a requirement will force
    /// the replication type (and give an error if requirements contradict each other).
    /// Otherwise the more common default will be used, with synchronous used in case of a tie.
    enum class ReplicationType {
      NO_PREFERENCE, REQUIRE_ASYNC, DEFAULT_ASYNC, DEFAULT_SYNC, REQUIRE_SYNC
    };
    ReplicationType rep_type = ReplicationType::NO_PREFERENCE;

    /// Which populations are we operating on?
    emp::reference_vector<mabe::Population> pops;  ///< Which population are we using?
    size_t min_pops = 0;                           ///< Minimum number of population needed
    size_t max_pops = 0;                           ///< Maximum number of population allowed
    bool default_pops_ok = true;                   ///< Is it okay to use default populations?

    /// Store information about organism traits that this module needs to work with.
    struct TraitInfo {
      std::string name="";
      std::string desc="";
      emp::TypeID type;

      /// Which modules are allowed to read or write this trait?
      enum Access {
        UNKNOWN,   ///< Access level unknown; most likely a problem!
        OWNED,     ///< Can READ & WRITE this trait; other modules can only read.
        SHARED,    ///< Can READ & WRITE this trait; other modules can too.
        REQUIRED,  ///< Can READ this trait, but another module must WRITE to it.
        PRIVATE    ///< Can READ & WRITE this trait.  Others cannot use it.
      };
      Access access = Access::UNKNOWN;

      /// How should this trait be initialized in a newly-born organism?
      /// * Injected organisms always use the default value.
      /// * Modules can moitor signals to make other changes at any time.
      enum Init {
        DEFAULT,   ///< Trait is initialized to a pre-set default value.
        PARENT,    ///< Trait is inhereted (from first parent if more than one)
        AVERAGE,   ///< Trait becomes average of all parents on birth.
        MINIMUM,   ///< Trait becomes lowest of all parents on birth.
        MAXIMUM    ///< Trait becomes highest of all parents on birth.
      };
      Init init = Init::DEFAULT;
      bool reset_parent = false;  ///< Should the parent ALSO be reset on birth?

      /// Which information should we store in the trait as we go?
      enum Archive {
        NONE,       ///< Don't store any older information.
        LAST_RESET, ///< Store value at reset in "last_(name)"
        ALL_RESET,  ///< Store values at all resets in "archive_(name)"
        ALL_CHANGE  ///< Store values from every change in "sequence_(name)"
        // @CAO: CHANGE not yet impements since hard to track...
      };
      Archive archive = Archive::NONE;

      /// How should these data be summarized in phyla types (such as Genotype)
      enum TypeRecord {
        IGNORE,         ///< Don't include this trait in phyla records.
        DEATH_AVERAGE,  ///< Store average value of organisms at their death.
        DEATH_SUMMARY,  ///< Store basic summary (min, max, count, ave) of orgs at death.
        DEATH_FULL,     ///< Store all values for organisms at their death.
        PARENT_AVERAGE, ///< Store average value of parents at organisms' births.
        PARENT_SUMMARY, ///< Store basic summary (min, max, count, ave) of parents at birth.
        PARENT_FULL     ///< Store all values for parents at organisms' births.
      };
      TypeRecord type_record = TypeRecord::IGNORE;

      /// Was a default value set for this trait (can only be done in overload that knows type)
      virtual bool HasDefault() { return false; }

      /// Set the current value of this trait to be automatically inthereted by offspring.
      TraitInfo & SetInheritParent() { init = Init::PARENT; return *this; }

      /// Set the average across parents for this trait to be automatically inthereted by offspring.
      TraitInfo & SetInheritAverage() { init = Init::AVERAGE; return *this; }

      /// Set the minimum across parents for this trait to be automatically inthereted by offspring.
      TraitInfo & SetInheritMinimum() { init = Init::MINIMUM; return *this; }

      /// Set the maximum across parents for this trait to be automatically inthereted by offspring.
      TraitInfo & SetInheritMaximum() { init = Init::MAXIMUM; return *this; }

      /// Set the parent to ALSO reset to the same value as the offspring on divide.
      TraitInfo & SetParentReset() { reset_parent = true; return *this; }

      /// Set the previous value of this trait to be stored on birth or reset.
      TraitInfo & SetArchiveLast() { archive = Archive::LAST_RESET; return *this; }

      /// Set ALL previous values of this trait to be store after each reset.
      TraitInfo & SetArchiveAll() { archive = Archive::ALL_RESET; return *this; }
    };

    template <typename T>
    struct TypedTraitInfo : public TraitInfo {
      T default_value;
      bool has_default;

      TypedTraitInfo(const std::string & in_name) : has_default(false)
      {
        name = in_name;
        type = emp::GetTypeID<T>();
      }

      TypedTraitInfo(const std::string & in_name, const T & in_default)
        : default_value(in_default), has_default(true)
      {
        name = in_name;
        type = emp::GetTypeID<T>();
      }

      bool HasDefault() { return has_default; }

      TypedTraitInfo<T> & SetDefault(const T & in_default) {
        default_value = in_default;
        has_default = true;
        return *this;
      }
    };

    emp::map<std::string, emp::Ptr<TraitInfo>> trait_map;

    // Helper functions
    template <typename... Ts>
    void AddError(Ts &&... args) {
      errors.push_back( emp::to_string( std::forward<Ts>(args)... ));
    }

  public:
    Module() : name("") { ; }
    Module(const Module &) = default;
    Module(Module &&) = default;
    virtual ~Module() { ; }

    const std::string & GetName() const noexcept { return name; }
    bool HasErrors() const { return errors.size(); }
    const emp::vector<std::string> & GetErrors() const noexcept { return errors; }
    size_t GetMinPops() const noexcept { return min_pops; }
    size_t GetMaxPops() const noexcept { return max_pops; }
    bool DefaultPopsOK() const noexcept { return default_pops_ok; }

    virtual emp::Ptr<Module> Clone() { return nullptr; }

    bool IsEvaluate() const noexcept  { return is_evaluate; }
    bool IsSelect() const noexcept  { return is_select; }
    bool IsPlacement() const noexcept  { return is_placement; }
    bool IsAnalyze() const noexcept  { return is_analyze; }

    Module & IsEvaluate(bool in) noexcept { is_evaluate = in; return *this; }
    Module & IsSelect(bool in) noexcept { is_select = in; return *this; }
    Module & IsPlacement(bool in) noexcept { is_placement = in; return *this; }
    Module & IsAnalyze(bool in) noexcept { is_analyze = in; return *this; }

    Module & RequireAsync() { rep_type = ReplicationType::REQUIRE_ASYNC; return *this; }
    Module & DefaultAsync() { rep_type = ReplicationType::DEFAULT_ASYNC; return *this; }
    Module & DefaultSync() { rep_type = ReplicationType::DEFAULT_SYNC; return *this; }
    Module & RequireSync() { rep_type = ReplicationType::REQUIRE_SYNC; return *this; }

    virtual void Setup(mabe::World &) { /* By default, assume no setup needed. */ }
    virtual void Update() { /* By default, do nothing at update. */ }

    // ------------ Population Management ------------

    size_t GetNumPops() const noexcept { return pops.size(); }

    const Population & GetPopulation(size_t id) const { return pops[id]; }

    /// Add an additional population to make use of.
    Module & UsePopulation( mabe::Population & in_pop ) {
      pops.push_back( in_pop );
      return *this;
    }

  // --------------------- Functions to be used in derived modules ONLY --------------------------
  protected:

    /// Set the number of populations that this module must work on.  If only one number is
    /// provided, that is the required number; if two that is the range.
    /// If both number are specified, may include a third arg to indivate if it is okat to use
    /// the default populations (which assumes the first population is "main" and the second, if
    /// there is one, is next generation.)
    void SetRequiredPops(size_t in_min, size_t in_max=0, bool in_default_ok=false) {
      emp_assert(in_max == 0 || in_max >= in_min);
      min_pops = in_min;
      max_pops = in_max;
      if (max_pops == 0) max_pops = min_pops;  // If maximum was not set, use minimum for both.
      default_pops_ok = in_default_ok;
    }

    // --== Trait management ==--
   
    /// Add a new trait to this module, specifying its access method, its name, and its description.
    template <typename T>
    TypedTraitInfo<T> & AddTrait(TraitInfo::Access access,
                                 const std::string & in_name,
                                 const std::string & desc) {
      if (emp::Has(trait_map, in_name)) {
        AddError("Module ", name, " is creating a duplicate trait named '", in_name, "'.");
      }
      auto new_ptr = emp::NewPtr<TypedTraitInfo<T>>(in_name, desc);
      new_ptr->access = access;
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
    TraitInfo & AddSharedTrait(const std::string & name, const std::string & desc) {
      return AddTrait<T>(TraitInfo::Access::SHARED, name, desc);
    }
    template <typename T>
    TraitInfo & AddSharedTrait(const std::string & name, const std::string & desc, const T & default_val) {
      return AddTrait<T>(TraitInfo::Access::SHARED, name, desc, default_val);
    }
   
    /// Add trait that this module can READ this trait, but another module must WRITE to it.
    template <typename T>
    TraitInfo & AddRequiredTrait(const std::string & name, const std::string & desc) {
      return AddTrait<T>(TraitInfo::Access::REQUIRED, name, desc);
    }


  };

}

#endif
