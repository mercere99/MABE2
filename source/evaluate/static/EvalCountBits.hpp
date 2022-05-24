/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  EvalCountBits.hpp
 *  @brief MABE Evaluation module for counting the number of ones (or zeros) in an output.
 */

#ifndef MABE_EVAL_COUNT_BITS_H
#define MABE_EVAL_COUNT_BITS_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

#include "emp/datastructs/reference_vector.hpp"

namespace mabe {

  class EvalCountBits : public Module {
  private:
    std::string bits_trait;
    std::string score_trait;
    bool count_type;   // =0 for counts zeros, or =1 for count ones.

  public:
    EvalCountBits(mabe::MABE & control,
                  const std::string & name="EvalCountBits",
                  const std::string & desc="Evaluate bitstrings by counting ones (or zeros).",
                  const std::string & _btrait="bits",
                  const std::string & _ftrait="score",
                  bool _ctype=1)
      : Module(control, name, desc)
      , bits_trait(_btrait)
      , score_trait(_ftrait)
      , count_type(_ctype)
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
      LinkVar(bits_trait, "bits_trait", "Which trait stores the bit sequence to evaluate?");
      LinkVar(score_trait, "score_trait", "Which trait should we store NK score in?");
      LinkVar(count_type, "count_type", "Which type of bit should we count? (0 or 1)");
    }

    void SetupModule() override {
      AddRequiredTrait<emp::BitVector>(bits_trait);
      AddOwnedTrait<double>(score_trait, "All-ones score value", 0.0);
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
        const emp::BitVector & bits = org.GetTrait<emp::BitVector>(bits_trait);
        double score = (double) bits.CountOnes();

        // If we were supposed to count zeros, subtract ones count from total number of bits.
        if (count_type == 0) score = bits.size() - score;

        // Store the count on the organism in the score trait.
        org.SetTrait<double>(score_trait, score);

        if (score > max_score || !max_org) {
          max_score = score;
          max_org = &org;
        }
      }

      std::cout << "Max " << score_trait << " = " << max_score << std::endl;
      return max_score;
    }
  };

  MABE_REGISTER_MODULE(EvalCountBits, "Evaluate bitstrings by counting ones (or zeros).");
}

#endif
