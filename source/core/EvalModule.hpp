/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2024.
 *
 *  @file  EvalModule.hpp
 *  @brief A module base class to simplify the creation of evaluation modules.
 */

#ifndef MABE_EVAL_MODULE_H
#define MABE_EVAL_MODULE_H

#include "emp/base/notify.hpp"

#include "MABE.hpp"
#include "Module.hpp"

namespace mabe {

  template <typename DERIVED_T>
  class EvalModule : public Module {
  public:
    EvalModule(mabe::MABE & control,
               emp::String name,
               emp::String desc)
      : Module(control, name, desc)
    {
      SetEvaluateMod(true);
    }
    ~EvalModule() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
                             [](DERIVED_T & mod, Collection list) { return mod.Evaluate(list); },
                             "Evaluate all orgs in the OrgList.");
      info.AddMemberFunction("RESET",
                             [](DERIVED_T & mod) { return mod.Reset(); },
                             "Regenerate the landscape with current config values.");
    }

    /// Run this evaluator on the provided collection.
    virtual double EvaluateCollection(const Collection & orgs) = 0;

    /// Run this evaluator on the provided collection.
    double Evaluate(const Collection & orgs) { return EvaluateCollection(orgs); };

    /// If a population is provided to Evaluate, first convert it to a Collection.
    double Evaluate(Population & pop) { return Evaluate( Collection(pop) ); }

    /// If a string is provided to Evaluate, convert it to a Collection.
    double Evaluate(const emp::String & in) { return Evaluate( control.ToCollection(in) ); }

    /// Re-randomize all of the entries.
    virtual double Reset() { emp::notify::Message("Module '", name, "' cannot be reset."); return 0.0;  }
  };

}

#endif
