/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  TraitInfo.h
 *  @brief Information about a single phenotypic trait.
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
    emp::Ptr<ModuleBase> owner;  ///< Pointer to owner module for trait (or creator for a shared trait)

  public:
    /// Which modules are allowed to read or write this trait?
    enum Access {
      UNKNOWN=0,   ///< Access level unknown; most likely a problem!
      PRIVATE,     ///< Can READ & WRITE this trait.  Others cannot use it.
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

    TraitInfo & SetDescription(std::string in_desc) { desc = in_desc; return *this; }
    TraitInfo & SetOwner(emp::Ptr<ModuleBase> in_owner) { owner = in_owner; return *this; }

    /// Was a default value set for this trait (can only be done in overload that knows type)
    virtual bool HasDefault() { return false; }
    bool DoResetParent() const { return reset_parent; }

    Access GetAccess() const { return access; }
    Init GetInit() const { return init; }
    Archive GetArchive() const { return archive; }
    TypeRecord GetTypeRecord() const { return type_record; }

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

}

#endif