/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  TraitInfo.hpp
 *  @brief Information about a single phenotypic trait.
 *
 *  A TraitInfo object contains basic information a about a single trait being tracked in an
 *  organism.  In addition to the name, type, and description of the trait, it also describes:
 *
 *  The TARGET indicates what type of object the trait should be applied to.
 *    [ORGANISM]   - Every organism in MABE must have this trait.
 *    [POPULATION] - Collections of organsims must have this trait.
 *    [MODULE]     - Every module attached to MABE must have this trait.
 *    [MANAGER]    - Every OrganismManager must have this trait.
 * 
 *  The ACCESS method to be used for a trait by each module.  A trait can be
 *    [PRIVATE]    - Only this module can modify the trait; no others should even read it.
 *    [OWNED]      - Only this module can modify the trait, but other modules can read it.
 *    [GENERATED]  - Only this module can modify the trait, but other modules MUST read it.
 *    [SHARED]     - This module will read and write this trait, but others are allowed to as well.
 *    [REQUIRED]   - This module will read the trait; another module must write to it.
 *    [OPTIONAL]   - This module can read the trait; must first check if it exists.
 *
 *  The INIT method describes how a trait should be initialized in a new offspring.
 *  (note that injected organisms always get the DEFAULT value.)
 *    [DEFAULT]    - Always initialize this trait to its default value.
 *    [FIRST]      - Initialize trait to the first parent's value (only parent for asexual runs)
 *    [AVERAGE]    - Initialize trait to the average value of all parents.
 *    [MINIMUM]    - Initialize trait to the minimum value of all parents.
 *    [MAXIMUM]    - Initialize trait to the maximum value of all parents.
 *
 *  The ARCHIVE method determines how many older values should be saved with each organism.
 *    [NONE]       - Only the most recent value should be tracked, no archived values.
 *    [AT_BIRTH]   - Store value of this trait was born with in "birth_(name)".
 *    [LAST_REPRO] - Store value of trait at the last reproduction in "last_(name)".
 *    [ALL_REPROS] - Store all value of trait at each reproduction event in "archive_(name)".
 *    [ALL_VALUES] - Store every value change of trait at any time in "sequence_(name)".
 *
 *  The SUMMARY method determines how a trait should be summarized over a collection of organisms.
 *    [[[ needs refinement... ]]]
 * 
 * 
 *  DEVELOPER NOTES:
 *
 *  There are two other more obscure ACCESS methods that may be worth implementing:
 *    [WEAK_SHARED] - Will write to a trait but others MUST also write to it.
 *    [SHARED_GENERATED] - Like generated, but others may also write to trait.
 */

#ifndef MABE_TRAIT_INFO_H
#define MABE_TRAIT_INFO_H

#include <set>
#include <string>

#include "emp/base/vector.hpp"
#include "emp/data/DataMap.hpp"
#include "emp/meta/TypeID.hpp"

namespace mabe {

  class ModuleBase;

  class TraitInfo {
  protected:
    std::string name="";                 ///< Unique name for this trait.
    std::string desc="";                 ///< Description of this trait.
    emp::TypeID type;                    ///< Type identifier for this trait.
    emp::vector<emp::TypeID> alt_types;  ///< What other types should be allowed?

  public:
    /// Which modules are allowed to read or write this trait?
    enum Access {
      UNKNOWN=0,   ///< Access level unknown; most likely a problem!
      PRIVATE,     ///< Can READ & WRITE this trait; other modules cannot use it at all.
      OWNED,       ///< Can READ & WRITE this trait; other modules can read it.
      GENERATED,   ///< Can READ & WRITE this trait; other modules MUST read it.
      SHARED,      ///< Can READ & WRITE this trait; other modules can too.
      REQUIRED,    ///< Can READ this trait, but another module must WRITE to it.
      OPTIONAL,    ///< Can READ this trait, but must check if it exists first.
      NUM_ACCESS   ///< How many access methods are there?
    };

    /// How should this trait be initialized (via inheritance) in a newly-born organism?
    /// * Injected organisms always use the default value.
    /// * Modules can moitor signals to make other changes at any time.
    enum class Init {
      DEFAULT=0, ///< Trait is initialized to a pre-set default value.
      FIRST,     ///< Trait is inhereted (from first parent if more than one)
      AVERAGE,   ///< Trait becomes average of all parents on birth.
      MINIMUM,   ///< Trait becomes lowest of all parents on birth.
      MAXIMUM,   ///< Trait becomes highest of all parents on birth.
      RANDOM     ///< Choose a random parent and use its value.
    };

    /// Which information should we store in the trait as we go?
    /// A "reproduction event" for an organism is when it's born and each time it gives birth.
    enum class Archive {
      NONE=0,     ///< Don't store any older information.
      AT_BIRTH,   ///< Store value this trait was born with in "birth_(name)"
      LAST_REPRO, ///< Store value at last reproduction event in "last_(name)"
      ALL_REPRO,  ///< Store values at all reproduction events (including birth) in "archive_(name)"
      ALL_VALUES  ///< Store values from every change in "sequence_(name)"
      // @CAO: ALL_VALUES can be hard to track...
    };

    /// What timings should we use when we summarize data?  (Must maintain summary!)
    enum class Timing {
      LATEST,     ///< Always use the most recent value set.
      PARENT,     ///< Use value of parent at time of organism birth.
      REPRO       ///< Use value last time organism replicated (or parent value if no births)
    };

    /// How should these data be summarized in groups such as whole population or phyla types
    /// (such as Genotype, Species, etc.)  Some traits shouldn't be summarized at all (IGNORE)
    /// Otherwise the summary values can be taken as:
    enum class Summary {
      IGNORE=0,   ///< Don't include this trait in phyla records.
      AVERAGE,    ///< Average of current value of all organisms (or final value at death).
      SUMMARY,    ///< Basic summary (min, max, count, ave) of current/final values.
      FULL,       ///< Store ALL current/final values for organisms.
    };

  protected:
    Init init = Init::DEFAULT;
    bool reset_parent = false;  ///< Should the parent ALSO be reset on birth?
    Archive archive = Archive::NONE;
    Summary summary = Summary::IGNORE;

    // Track which modules are using this trait and what access they need.
    using mod_ptr_t = emp::Ptr<ModuleBase>;
    struct ModuleInfo {
      std::string mod_name = "";
      mod_ptr_t mod_ptr = nullptr;
      Access access = Access::UNKNOWN;
      bool is_manager = false;
    };
    emp::vector<ModuleInfo> access_info;

    // Specific access categories
    emp::array<size_t, NUM_ACCESS> access_counts = { 0, 0, 0, 0, 0, 0, 0 };
    emp::array<size_t, NUM_ACCESS> manager_access_counts = { 0, 0, 0, 0, 0, 0, 0 };

    // Helper functions
    int GetInfoID(const std::string & mod_name) const {
      for (int i = 0; i < (int) access_info.size(); i++) {
        if (access_info[(size_t) i].mod_name == mod_name) return i;
      }
      return -1;
    }

    int GetInfoID(mod_ptr_t mod_ptr) const {
      for (int i = 0; i < (int) access_info.size(); i++) {
        if (access_info[(size_t) i].mod_ptr == mod_ptr) return i;
      }
      return -1;
    }

  public:
    virtual ~TraitInfo() { ; }

    const std::string & GetName() const { return name; }
    const std::string & GetDesc() const { return desc; }
    emp::TypeID GetType() const { return type; }
    const emp::vector<emp::TypeID> & GetAltTypes() const { return alt_types; }

    template <typename... Ts>
    void SetAltTypes(const emp::vector<emp::TypeID> & in_alt_types) { alt_types = in_alt_types; }
    template <typename T> bool IsType() const { return GetType() == emp::GetTypeID<T>(); }
    bool IsAllowedType(emp::TypeID test_type) const { return Has(alt_types, test_type); };
    template <typename T> bool IsAllowedType() const { return IsAllowedType(emp::GetTypeID<T>()); }

    /// Determine what kind of access a module has.
    Access GetAccess(mod_ptr_t mod_ptr) const {
      int id = GetInfoID(mod_ptr);
      if (id == -1) return Access::UNKNOWN;
      return access_info[id].access;
    }

    /// Determine if a module has any knd of access to this trait.
    bool HasAccess(mod_ptr_t mod_ptr) const { return GetAccess(mod_ptr) != Access::UNKNOWN; }

    /// How many modules can access this trait?
    size_t GetModuleCount() const { return access_info.size(); }

    /// How many modules can access this trait using a specified access mode?
    size_t GetAccessCount(Access access) const { return access_counts[access]; }

    bool IsPrivate() const { return GetAccessCount(Access::PRIVATE); }
    bool IsOwned() const { return GetAccessCount(Access::OWNED); }
    bool IsGenerated() const { return GetAccessCount(Access::GENERATED); }
    bool IsShared() const { return GetAccessCount(Access::SHARED); }
    bool IsRequired() const { return GetAccessCount(Access::REQUIRED); }
    bool IsOptional() const { return GetAccessCount(Access::OPTIONAL); }

    size_t GetUnknownCount() const { return GetAccessCount(Access::UNKNOWN); }
    size_t GetPrivateCount() const { return GetAccessCount(Access::PRIVATE); }
    size_t GetOwnedCount() const { return GetAccessCount(Access::OWNED); }
    size_t GetGeneratedCount() const { return GetAccessCount(Access::GENERATED); }
    size_t GetSharedCount() const { return GetAccessCount(Access::SHARED); }
    size_t GetRequiredCount() const { return GetAccessCount(Access::REQUIRED); }
    size_t GetOptionalCount() const { return GetAccessCount(Access::OPTIONAL); }

    emp::vector<std::string> GetModuleNames() const {
      emp::vector<std::string> mod_names;
      for (auto info : access_info) {
        mod_names.push_back(info.mod_name);
      }
      return mod_names;
    }

    emp::vector<std::string> GetModuleNames(Access test_access) const {
      emp::vector<std::string> mod_names;
      for (auto info : access_info) {
        if (info.access == test_access) mod_names.push_back(info.mod_name);
      }
      return mod_names;
    }

    emp::vector<std::string> GetUnknownNames() const { return GetModuleNames(Access::UNKNOWN); }
    emp::vector<std::string> GetPrivateNames() const { return GetModuleNames(Access::PRIVATE); }
    emp::vector<std::string> GetOwnedNames() const { return GetModuleNames(Access::OWNED); }
    emp::vector<std::string> GetGeneratedNames() const { return GetModuleNames(Access::GENERATED); }
    emp::vector<std::string> GetSharedNames() const { return GetModuleNames(Access::SHARED); }
    emp::vector<std::string> GetRequiredNames() const { return GetModuleNames(Access::REQUIRED); }
    emp::vector<std::string> GetOptionalNames() const { return GetModuleNames(Access::OPTIONAL); }

    /// Was a default value set for this trait (can only be done in overload that knows type)
    virtual bool HasDefault() const { return false; }
    bool GetResetParent() const { return reset_parent; }

    Init GetInit() const { return init; }
    Archive GetArchive() const { return archive; }
    Summary GetSummary() const { return summary; }

    TraitInfo & SetName(const std::string & in_name) { name = in_name; return *this; }
    TraitInfo & SetDesc(const std::string & in_desc) { desc = in_desc; return *this; }
 
    // Add a module that can access this trait.
    TraitInfo & AddAccess(const std::string & in_name, mod_ptr_t in_mod, Access access, bool is_manager) {
      access_info.push_back(ModuleInfo{ in_name, in_mod, access, is_manager });
      access_counts[access]++;
      if (is_manager) manager_access_counts[access]++;
      return *this;
    }

    /// Set the current value of this trait to be automatically inthereted by offspring.
    TraitInfo & SetInheritParent() { init = Init::FIRST; return *this; }

    /// Set the average across parents for this trait to be automatically inthereted by offspring.
    TraitInfo & SetInheritAverage() { init = Init::AVERAGE; return *this; }

    /// Set the minimum across parents for this trait to be automatically inthereted by offspring.
    TraitInfo & SetInheritMinimum() { init = Init::MINIMUM; return *this; }

    /// Set the maximum across parents for this trait to be automatically inthereted by offspring.
    TraitInfo & SetInheritMaximum() { init = Init::MAXIMUM; return *this; }

    /// Set the parent to ALSO reset to the same value as the offspring on divide.
    TraitInfo & SetParentReset() { reset_parent = true; return *this; }

    /// Set the previous value of this trait to be stored on birth or reset.
    TraitInfo & SetArchiveLast() { archive = Archive::LAST_REPRO; return *this; }

    /// Set ALL previous values of this trait to be store after each reset.
    TraitInfo & SetArchiveAll() { archive = Archive::ALL_REPRO; return *this; }

    /// Register this trait in the provided DataMap.
    virtual void Register(emp::DataMap & dm) const = 0;
  };

  // Information about this trait, including type information and alternate type options.
  template <typename T>
  class TypedTraitInfo : public TraitInfo {
  private:
    T default_value;
    bool has_default;

  public:
    TypedTraitInfo(const std::string & in_name="") : has_default(false)
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

    bool HasDefault() const override { return has_default; }

    const T & GetDefault() const { return default_value; }

    TypedTraitInfo<T> & SetDefault(const T & in_default) {
      default_value = in_default;
      has_default = true;
      return *this;
    }
    
    void Register(emp::DataMap & dm) const override {
      dm.AddVar(name, default_value, desc);
    }

  };

}

#endif
