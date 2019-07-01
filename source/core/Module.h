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

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/reference_vector.h"
#include "tools/vector_utils.h"

namespace mabe {

  class MABE;
  class World;

  class Module {
  protected:
    std::string name;

    // What type of module is this (note, some can be more than one!)
    bool is_evaluate=false;   ///< Does this module perform evaluation on organisms?
    bool is_select=false;     ///< Does this module select organisms to reproduce?
    bool is_placement=false;  ///< Does this module handle offspring placement?
    bool is_analyze=false;    ///< Does this module record or evaluate data?

    enum class ReplicationType {
      NO_PREFERENCE, REQUIRE_ASYNC, DEFAULT_ASYNC, DEFAULT_SYNC, REQUIRE_SYNC
    };
    ReplicationType rep_type = ReplicationType::NO_PREFERENCE;

    /// Which populations are we operating on?
    emp::reference_vector<mabe::Population> pops;  ///< Which population are we using?
    size_t required_pops = 0;                      ///< How many population are needed?

    /// Store information about organism traits that this module needs to work with.
    struct TraitInfo {
      std::string name="";

      /// Which modules are allowed to read or write this trait?
      enum Access {
        UNKNOWN,   ///< Access level unknown; most likely a problem!
        OWNED,     ///< Can READ & WRITE this trait; other modules can only read.
        SHARED,    ///< Can READ & WRITE this trait; other modules can too.
        REQUIRED,  ///< Can READ this trait, but another module must WRITE to it.
        PRIVATE    ///< Can READ & WRITE this trait.  Others cannot use it.
      };
      Access access = Access::UNKNOWN;

      /// When should this trait be reset to either the default or a specified value?
      enum Reset {
        NEVER,      ///< Trait is inhereted (from first parent) and never automatically reset
        ON_BIRTH,   ///< Trait is only reset when an organism is first born.
        ON_DIVIDE,  ///< Trait is reset on birth and when giving birth.
        TO_AVERAGE, ///< Trait becomes average of all parents on birth.
        TO_MIN,     ///< Trait becomes lowest of all parents on birth.
        TO_MAX      ///< Trait becomes highest of all parents on birth.
      };
      Reset reset = Reset::NEVER;

      /// Which information should we store in the trait as we go?
      enum Archive {
        NONE,       ///< Don't store any older information.
        LAST_RESET, ///< Store value at reset in "last_(name)"
        ALL_RESET,  ///< Store values at all resets in "archive_(name)"
        ALL_CHANGE  ///< Store values from every change in "archive_(name)"
      };
      Archive archive = Archive::NONE;

    };


  public:
    Module() : name("") { ; }
    Module(const Module &) = default;
    Module(Module &&) = default;
    virtual ~Module() { ; }

    const std::string & GetName() const { return name; }

    virtual emp::Ptr<Module> Clone() { return nullptr; }

    bool IsEvaluate() const { return is_evaluate; }
    bool IsSelect() const { return is_select; }
    bool IsPlacement() const { return is_placement; }
    bool IsAnalyze() const { return is_analyze; }

    Module & IsEvaluate(bool in) noexcept { is_evaluate = in; return *this; }
    Module & IsSelect(bool in) noexcept { is_select = in; return *this; }
    Module & IsPlacement(bool in) noexcept { is_placement = in; return *this; }
    Module & IsAnalyze(bool in) noexcept { is_analyze = in; return *this; }

    Module & RequireAsync() { rep_type = ReplicationType::REQUIRE_ASYNC; return *this; }
    Module & DefaultAsync() { rep_type = ReplicationType::DEFAULT_ASYNC; return *this; }
    Module & DefaultSync() { rep_type = ReplicationType::DEFAULT_SYNC; return *this; }
    Module & RequireSync() { rep_type = ReplicationType::REQUIRE_SYNC; return *this; }

    // Internal, initial setup.
    void InternalSetup(mabe::World & world) {
      // World needs to check if all populations are setup correctly.
    }

    virtual void Setup(mabe::World &) { /* By default, assume no setup needed. */ }
    virtual void Update() { /* By default, do nothing at update. */ }

    /// Add an additional population to evaluate.
    Module & AddPopulation( mabe::Population & in_pop ) {
      pops.push_back( in_pop );
      return *this;
    }

  };

}

#endif
