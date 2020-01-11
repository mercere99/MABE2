/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  TraitInfo.h
 *  @brief Information about a single phenotypic trait.
 *
 *  A TraitInfo object contains basic information a about a single trait being tracked in an
 *  organism.  In addition to the name, type, and description of the trait, it also describes:

 *  The ACCESS method to be used for a trait by each module.  A trait can be
 *    [PRIVATE]  - Only this module can access it; no others should read or write it.
 *    [OWNED]    - Only this module can modify the trait, but other modules can read it.
 *    [SHARED]   - This module will read and write this trait, but others are allowed to as well.
 *    [REQUIRED] - This module will read the trait; another module must write to it.
 *
 *  The INIT method describes how a trait should be initialized in a new offspring.
 *  (note that injected organisms always get the DEFAULT value.)
 *    [DEFAULT] - Always initialize this trait to its default value.
 *    [FIRST]   - Initialize trait to the first parent's value (only parent for asexual runs)
 *    [AVERAGE] -  Initiialize trait to the average value of all parents.
 *    [MINIMUM] -  Initiialize trait to the minimum value of all parents.
 *    [MAXIMUM] -  Initiialize trait to the maximum value of all parents.
 *
 */

#ifndef MABE_TRAIT_INFO_H
#define MABE_TRAIT_INFO_H

#include <string>

#include "meta/TypeID.h"

namespace mabe {

  class ModuleBase;

  class TraitInfo {
  protected:
    std::string name="";         ///< Unique name for this trait.
    std::string desc="";         ///< Description of this trait.
    emp::TypeID type;            ///< Type identifier for this trait.

    using mod_ptr_t = emp::Ptr<ModuleBase>;
    emp::vector<mod_ptr_t> users;  ///< Pointer to all modules that use this trait.

  public:
    /// Which modules are allowed to read or write this trait?
    enum Access {
      UNKNOWN=0,   ///< Access level unknown; most likely a problem!
      PRIVATE,     ///< Can READ & WRITE this trait; other modules cannot use it at all.
      OWNED,       ///< Can READ & WRITE this trait; other modules can only read.
      SHARED,      ///< Can READ & WRITE this trait; other modules can too.
      REQUIRED,    ///< Can READ this trait, but another module must WRITE to it.
      NUM_ACCESS   ///< How many access methods are there?
    };

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

    /// Which information should we store in the trait as we go?
    enum Archive {
      NONE,       ///< Don't store any older information.
      LAST_RESET, ///< Store value at reset in "last_(name)"
      ALL_RESET,  ///< Store values at all resets in "archive_(name)"
      ALL_CHANGE  ///< Store values from every change in "sequence_(name)"
      // @CAO: CHANGE not yet impements since hard to track...
    };

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

  protected:
    Access access = Access::UNKNOWN;
    Init init = Init::DEFAULT;
    bool reset_parent = false;  ///< Should the parent ALSO be reset on birth?
    Archive archive = Archive::NONE;
    TypeRecord type_record = TypeRecord::IGNORE;

  public:
    virtual ~TraitInfo() { ; }

    const std::string & GetName() const { return name; }
    const std::string & GetDesc() const { return desc; }
    emp::TypeID GetType() const { return type; }
    const emp::vector<mod_ptr_t> & GetUsers() const { return users; }

    /// Was a default value set for this trait (can only be done in overload that knows type)
    virtual bool HasDefault() { return false; }
    bool GetResetParent() const { return reset_parent; }

    Access GetAccess() const { return access; }
    Init GetInit() const { return init; }
    Archive GetArchive() const { return archive; }
    TypeRecord GetTypeRecord() const { return type_record; }

    TraitInfo & SetName(const std::string & in_name) { name = in_name; return *this; }
    TraitInfo & SetDesc(const std::string & in_desc) { desc = in_desc; return *this; }
    TraitInfo & AddUser(mod_ptr_t in_mod) { users.push_back(in_mod); return *this; }

    /// Set the access level of this trait to a specified level.
    TraitInfo & SetAccess(Access in_access) { access = in_access; return *this; }

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

    bool HasDefault() { return has_default; }

    const T & GetDefault() const { return default_value; }

    TypedTraitInfo<T> & SetDefault(const T & in_default) {
      default_value = in_default;
      has_default = true;
      return *this;
    }
  };

}

#endif