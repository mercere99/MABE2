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
    emp::reference_vector<mabe::Population> pops;
    size_t required_pops = 0;                     ///< How many population are needed?

    // @CAO Also need to track which traits are:
    //       OWNED    - This module can READ & WRITE this trait.  Others can only READ.
    //       SHARED   - This module can READ & WRITE this trait, but others can also READ & WRITE.
    //       REQUIRED - This module can READ this trait, but another must WRITE to it.
    //       PRIVATE  - This module can READ & WRITE this trait.  Others cannot use it.
    //
    //      Each trait also need information on how it will be monitored and inhereted.
    //       LOGGED      - Track all of the values placed in here for later analysis (don't overwrite)
    //       INHERETED   - Offspring have this trait initilized to parent's value (vs. using default).
    //       RECORD_LAST - When reproducing keep the previous value.
    //       RESET_BIRTH - Reset to default after giving birth.
    //
    //      Also include information on whether/how each trait should be archived.

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
