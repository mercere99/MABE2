/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2021.
 *
 *  @file  EvalMatchBits.hpp
 *  @brief MABE Evaluation module for counting the number of bits that MATCH with another organism.
 * 
 * 
 *  DEVELOPER NOTES:
 *  - We should allow offsets, skips, etc, to do more sophisticated pairings for matches.
 */

#ifndef MABE_EVAL_MATCH_BITS_H
#define MABE_EVAL_MATCH_BITS_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

#include "emp/datastructs/reference_vector.hpp"

namespace mabe {

  class EvalMatchBits : public Module {
  private:
    enum Type {
      MATCH_COUNT,
      MISMATCH_COUNT,
      UNKNOWN
    };

    std::string bits_trait = "bits";
    std::string score_trait = "bit_matches";
    Type match_type = Type::MATCH_COUNT;
    bool record_both = false;             // Save result on both organisms? (vs. first only)
    double empty_score = 0.0;             // Score to give orgs matched with empty positions.

  public:
    EvalMatchBits(mabe::MABE & control,
                  const std::string & name="EvalMatchBits",
                  const std::string & desc="Evaluate org bitstring by counting matches with another org's bisstring.")
      : Module(control, name, desc)
    {
      SetEvaluateMod(true);
    }
    ~EvalMatchBits() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
                             [](EvalMatchBits & mod, Collection list1, Collection list2) {
                               return mod.Evaluate(list1, list2);
                              },
                             "Evaluate Bit Matching by comparing orgs in the two OrgLists.");
    }

    void SetupConfig() override {
      LinkVar(bits_trait, "bits_trait", "Trait storing bit sequence to evaluate.");
      LinkVar(score_trait, "score_trait", "Trait to store match score result.");
      LinkMenu(match_type, "match_type", "How should the bit sequences be compared?",
        Type::MATCH_COUNT, "match_count", "Count bit positions with the same value.",
        Type::MISMATCH_COUNT, "mismatch_count", "Count bit positions with the different values.");
      LinkVar(record_both, "record_both", "Save result on both organisms? (0 -> first only)");
      LinkVar(empty_score, "empty_score", "Score to give orgs matched again an empty position?");
    }

    void SetupModule() override {
      AddRequiredTrait<emp::BitVector>(bits_trait);
      AddOwnedTrait<double>(score_trait, "Match score value", 0.0);
    }

    double EvaluateMatch(Organism & org1, Organism & org2) {      
      double match_score = empty_score;

      // Only calculate a real score if both organisms are non-empty.
      if (!org1.IsEmpty() && !org2.IsEmpty()) {
        // Make sure both organisms have bit sequences ready for us to access.
        org1.GenerateOutput();
        org2.GenerateOutput();

        const emp::BitVector & bits1 = org1.GetTrait<emp::BitVector>(bits_trait);
        const emp::BitVector & bits2 = org2.GetTrait<emp::BitVector>(bits_trait);
        org1.SetTrait<double>(score_trait, match_score);

        // Count the number of matches in the bit sequences.
        switch (match_type) {
          case Type::MATCH_COUNT:
            match_score = (double) (bits1 ^ bits2).CountZeros();
            break;
          case Type::MISMATCH_COUNT:          
            match_score = (double) (bits1 ^ bits2).CountOnes();
            break;
          default:
            emp_error("Unknown match type for EvalMatchBits!");
            match_score = -1.0;
        }
      }

      if (!org1.IsEmpty()) {
        org1.SetTrait<double>(score_trait, match_score);
      }
      if (record_both && !org2.IsEmpty()) {
        org2.SetTrait<double>(score_trait, match_score);
      }

      return match_score;
    }


    double Evaluate(Collection orgs1, Collection orgs2) {
      emp_assert(control.GetNumPopulations() >= 1);

      // Loop through the populations and evaluate each organism pair.
      double best_match = 0.0;

      // @CAO Should be a user-level error.
      emp_assert (orgs1.GetSize() == orgs2.GetSize(),
                  "EvalMatchBits::Evaluate requires two OrgLists of the same size.");

      auto it1 = orgs1.begin();
      auto it2 = orgs2.begin();
      while (it1 != orgs1.end()) {
        const double match = EvaluateMatch(*it1, *it2);
        if (match > best_match) best_match = match;
        ++it1; ++it2;
      }

      return best_match;
    }
  };

  MABE_REGISTER_MODULE(EvalMatchBits, "Evaluate bitstrings based on how well they match other organisms.");
}

#endif
