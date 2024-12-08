/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2024.
 *
 *  @file  EvalCountBits.hpp
 *  @brief MABE Evaluation module for counting the number of ones (or zeros) in an output.
 */

#ifndef MABE_EVAL_COUNT_BITS_H
#define MABE_EVAL_COUNT_BITS_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

#include "emp/datastructs/reference_vector.hpp"
#include "emp/tools/String.hpp"

namespace mabe {

  class EvalCountBits : public Module {
  private:
    RequiredTrait<emp::BitVector> bits_trait{this, "bits", "Bit-sequence to evaluate."};
    OwnedTrait<double> score_trait{this, "score", "Count of the number of specified bits"};

    bool count_type;   // =0 for counts zeros, or =1 for count ones.

  public:
    EvalCountBits(mabe::MABE & control,
                  emp::String name="EvalCountBits",
                  emp::String desc="Evaluate bitstrings by counting ones (or zeros).")
      : Module(control, name, desc)
    {
      SetEvaluateMod(true);
    }
    ~EvalCountBits() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
                             [](EvalCountBits & mod, Collection list) { return mod.Evaluate(list); },
                             "Count the ones in all orgs in an OrgList.");
    }

    void SetupConfig() override {
      LinkVar(count_type, "count_type", "Which type of bit should we count? (0 or 1)");
    }

    void SetupModule() override {
      // Nothing needed for now.
    }

    double Evaluate(Collection orgs) {
      emp_assert(control.GetNumPopulations() >= 1);

      // Loop through the population and evaluate each organism.
      double max_score = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_collect( orgs.GetAlive() );
      for (Organism & org : alive_collect) {        
        // Make sure this organism has its bit sequence ready for us to access.
        org.GenerateOutput();

        // Count the number of ones in the bit sequence.
        const emp::BitVector & bits = bits_trait.Get(org);
        double score = (double) bits.CountOnes();

        // If we were supposed to count zeros, subtract ones count from total number of bits.
        if (count_type == 0) score = bits.size() - score;

        // Store the count on the organism in the score trait.
        score_trait(org) = score;

        if (score > max_score || !max_org) {
          max_score = score;
          max_org = &org;
        }
      }

      std::cout << "Max " << score_trait.GetName() << " = " << max_score << std::endl;
      return max_score;
    }
  };

  MABE_REGISTER_MODULE(EvalCountBits, "Evaluate bitstrings by counting ones (or zeros).");
}

/*
== New Version:

module EvalCountBits {
  desc: "Count the number of ones in a bitsequence.",
  module_type: "evaluation"

  trait bits : BitSet {
    access: "required";
    default_name: "bits";
    desc: "bit sequence to evaluate";
  }
  trait score : UInt {
    access: "owned";
    default: 0;
    default_name: "score";
    desc: "Count of the number of specified bits";
  }
  config count_type : Bool {
    default: 1;
    desc: "Which type of bit should we count? (0 or 1)";
  }

  function(OrgSet orgs {desc: "Organisms to evaluate"}) : UInt {
    desc: "Count the number of ones in each organism's bitsequence.";

    UInt max_score = 0;
    for org in orgs {
      org.GenerateOutput();
      score(org) = count_type ? bits(org).CountOnes() : bits(org).CountZeros();
      if (score(org) > max_score) max_score = score(org);
    }
    return max_score;
  }
}

*/

#endif
