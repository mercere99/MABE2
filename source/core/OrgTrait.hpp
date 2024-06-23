/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2024.
 *
 *  @file  OrgTrait.hpp
 *  @brief Handles organism trait accessing for modules.
 */

#ifndef MABE_ORG_TRAIT_HPP
#define MABE_ORG_TRAIT_HPP

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/tools/String.hpp"

#include "ModuleBase.hpp"
#include "TraitInfo.hpp"

namespace mabe {

  /// AsConfig() provides a simple mechanism for indicating that you want to store a reference to a variable
  /// (because it will be set during configuration) rather than simply using a static value.
  template <typename T>
  struct ConfigPlaceholder {
    T & var;
    ConfigPlaceholder(T & in_var) : var(in_var) { }
    ConfigPlaceholder(const ConfigPlaceholder &) = default;
  };

  // -- Trait information for use in derived module types. --
  class BaseTrait {
  protected:
    using Access = TraitInfo::Access;

    Access access = Access::UNKNOWN;          ///< Which modules can read/write this trait?
    bool multi = false;                       ///< Can this trait have multiple values?
    emp::Ptr<ModuleBase> module_ptr=nullptr;  ///< Track which module owns this trait object.
    emp::Ptr<TraitHolder> held_ptr=nullptr;   ///< Track which object created this trait.
    emp::String name;                         ///< Name of this trait in a DataMap
    emp::String desc;                         ///< Description of this trait
    size_t count = 0;                         ///< Number of entries used for this trait.
    emp::Ptr<size_t> count_ref=nullptr;       ///< If count determined by config var, store here.

    emp::String config_name;                  ///< Trait name in config file.
    emp::String config_desc;                  ///< Description for trait name in config file.
    size_t id = emp::MAX_SIZE_T;              ///< ID of this trait in the DataMap.

  public:
    BaseTrait(Access _a, bool _m, emp::Ptr<TraitHolder> _hp, const emp::String & _n,
              const emp::String & _d="", size_t _c=1)
      : access(_a), multi(_m), held_ptr(_hp), name(_n), desc(_d), count(_c),
        config_name(name + "_trait"), config_desc(emp::String("Trait name for ") + _d)
    {
      emp_assert(multi == true || count == 1, multi, count);
      emp_assert(held_ptr);
      held_ptr->trait_ptrs.push_back(this);   // Store trait for lookup in module.

      // If the held pointer is a module, copy it over.  Otherwise set to nullptr.
      module_ptr = held_ptr.DynamicCast<ModuleBase>(); 
    }

    /// Constructor for when trait count is determined by a different config variable.
    /// Trait treated as a multi-trait, but we don't know specific count until after configuration.
    BaseTrait(Access _a, bool _m, emp::Ptr<TraitHolder> _hp, const emp::String & _n,
              const emp::String & _d, ConfigPlaceholder<size_t> _cr)
      : BaseTrait(_a, _m, _hp, _n, _d, 0) { count_ref = &_cr.var; }


    virtual ~BaseTrait() { }

    Module & GetModule() { return *(module_ptr.DynamicCast<Module>()); }
    Access GetAccess() const { return access; }
    const emp::String & GetName() const { return name; }
    const emp::String & GetDesc() const { return desc; }
    size_t GetCount() const { return count_ref ? *count_ref : count; }
    const emp::String & GetConfigName() const { return config_name; }
    const emp::String & GetConfigDesc() const { return config_desc; }
    size_t GetID() const { return id; }

    emp::String & GetNameVar() { return name; }

    virtual void AddTrait() = 0;
    void SetModule(emp::Ptr<ModuleBase> in_mod) { module_ptr = in_mod; }
    void SetName(const emp::String & _name) { name = _name; }
    void SetConfigName(const emp::String & _name) { config_name = _name; }
    void SetConfigDesc(const emp::String & _desc) { config_desc = _desc; }
    void SetupDataMap(const emp::DataMap & dm) { id = dm.GetID(name); }

    virtual bool ReadOK() const = 0;
    virtual bool WriteOK() const = 0;
    virtual bool OtherReadOK() const = 0;
    virtual bool OtherWriteOK() const = 0;
  };

  /// Extension on BaseTrait to allow typed operations including saving of default value.
  template <typename T,                  // What type is this trait?
            TraitInfo::Access ACCESS,    // How can this trait be accessed?
            bool MULTI>                  // Does this trait have multiple values?
  struct OrgTrait : public BaseTrait {
    T default_value{};

    // The get type should be a T & for a single value or a span<T> for multiple values.
    using get_t = typename std::conditional<MULTI, std::span<T>, T &>::type;
    using const_get_t = typename std::conditional<MULTI, std::span<const T>, const T &>::type;

    template<typename... Ts>
    OrgTrait(Ts &&... args) : BaseTrait(ACCESS, MULTI, std::forward<Ts>(args)...) { }

    /// Get() takes an organism and returns the trait reference for that organism.
    get_t Get(mabe::Organism & org) const {
      if constexpr (MULTI) return org.GetTrait<T>(id, GetCount());
      else return org.GetTrait<T>(id);
    }

    /// Get() takes a const organism and returns the trait value for that organism.
    const_get_t Get(const mabe::Organism & org) const {
      if constexpr (MULTI) return org.GetTrait<T>(id, GetCount());
      else return org.GetTrait<T>(id);
    }

    /// A trait supplied with an organism converts to the trait reference for that organism.
    inline get_t operator()(mabe::Organism & org) const { return Get(org); }

    /// A trait supplied with a const organism converts to the trait value for that organism.
    inline const_get_t operator()(const mabe::Organism & org) const { return Get(org); }

    // /// A trait supplied with an organism converts to the trait reference for that organism.
    // inline get_t operator()(emp::Ptr<mabe::Organism> org_ptr) const { return Get(*org_ptr); }

    // /// A trait supplied with a const organism converts to the trait value for that organism.
    // inline const_get_t operator()(emp::Ptr<const mabe::Organism> org_ptr) const { return Get(*org_ptr); }

    /// If a trait is supplied a collection it returns a vector of values, one for each
    /// organism in the collection.
    auto operator()(mabe::Collection & collect) const {
      const size_t num_orgs = collect.GetSize();
      emp::vector<T> out_v;
      out_v.reserve(num_orgs);
      for (auto & org : collect) {
        out_v.push_back(org.GetTrait<T>(id,GetCount()));
      }
      return out_v;
    }

    /// Adjust the default value associated with this trait.
    void SetDefault(T _default) { default_value = _default; }

    /// Internal mechanism for this trait to be added to the module.
    //  @CAO: Needs cleanup and better integration with TraitInfo.
    void AddTrait() override {
      emp_assert(module_ptr, "Internal error - module pointer should have been set before AddTrait is called.");
      module_ptr->GetTraitManager().AddTrait<T>(module_ptr, ACCESS, name, desc, default_value, GetCount());
    }

    bool ReadOK() const override {
      return true;
    }
    bool WriteOK() const override {
      if constexpr (ACCESS == Access::REQUIRED || ACCESS == Access::OPTIONAL) return false;
      return true;
    }
    bool OtherReadOK() const override {
      if constexpr (ACCESS == Access::PRIVATE) return false;
      return true;
    }
    bool OtherWriteOK() const override {
      if constexpr (ACCESS == Access::PRIVATE || ACCESS == Access::OWNED || ACCESS == Access::GENERATED) return false;
      return true;
    }
  };

  /// Special-case trait that can have any base type, but get's converted to string when accessed.
  struct RequiredTraitAsString : public BaseTrait {
    template<typename... Ts>
    RequiredTraitAsString(emp::Ptr<TraitHolder> held_ptr, const emp::String & name, const emp::String & desc="")
      : BaseTrait(TraitInfo::Access::REQUIRED, false, held_ptr, name, desc) { }

    /// Get() takes an organism and returns the trait reference for that organism.
    emp::String Get(mabe::Organism & org) const { return org.GetTraitAsString(id); }

    /// A trait supplied with an organism converts to the trait reference for that organism.
    inline emp::String operator()(mabe::Organism & org) const { return Get(org); }

    /// If a trait is supplied a collection it returns a vector of values, one for each
    /// organism in the collection.
    emp::vector<emp::String> operator()(mabe::Collection & collect) const {
      const size_t num_orgs = collect.GetSize();
      emp::vector<emp::String> out_v;
      out_v.reserve(num_orgs);
      for (auto & org : collect) {
        out_v.push_back(org.GetTraitAsString(id));
      }
      return out_v;
    }

    /// Internal mechanism for this trait to be added to the module.
    void AddTrait() override {
      emp_assert(module_ptr, "Internal error - module pointer should have been set before AddTrait is called.");
      module_ptr->GetTraitManager().AddTraitAsString(module_ptr, name);
    }

    bool ReadOK() const override { return true; }
    bool WriteOK() const override {return false; }
    bool OtherReadOK() const override { return true; }
    bool OtherWriteOK() const override { return true; }
  };

  template <typename T> ConfigPlaceholder<T> AsConfig(T & in_var) { return in_var; }

  // Traits that are read- and write-protected.
  template <typename T, bool MULTI=false> using PrivateTrait = OrgTrait<T, TraitInfo::Access::PRIVATE, MULTI>;
  template <typename T> using PrivateMultiTrait = OrgTrait<T, TraitInfo::Access::PRIVATE, true>;

  // Traits that are world readable, but write-protected
  template <typename T, bool MULTI=false> using OwnedTrait = OrgTrait<T, TraitInfo::Access::OWNED, MULTI>;
  template <typename T> using OwnedMultiTrait = OrgTrait<T, TraitInfo::Access::OWNED, true>;

  // Traits that are write-protected but MUST be read by another.
  template <typename T, bool MULTI=false> using GeneratedTrait = OrgTrait<T, TraitInfo::Access::GENERATED, MULTI>;
  template <typename T> using GeneratedMultiTrait = OrgTrait<T, TraitInfo::Access::GENERATED, true>;

  // Traits that can be freely shared (read or write) with others.
  template <typename T, bool MULTI=false> using SharedTrait = OrgTrait<T, TraitInfo::Access::SHARED, MULTI>;
  template <typename T> using SharedMultiTrait = OrgTrait<T, TraitInfo::Access::SHARED, true>;

  // Traits that must be generated by another.
  template <typename T, bool MULTI=false> using RequiredTrait = OrgTrait<T, TraitInfo::Access::REQUIRED, MULTI>;
  template <typename T> using RequiredMultiTrait = OrgTrait<T, TraitInfo::Access::REQUIRED, true>;

  // Traits that will be used if created by another, but are not required.
  template <typename T, bool MULTI=false> using OptionalTrait = OrgTrait<T, TraitInfo::Access::OPTIONAL, MULTI>;
  template <typename T> using OptionalMultiTrait = OrgTrait<T, TraitInfo::Access::OPTIONAL, true>;

}

#endif
