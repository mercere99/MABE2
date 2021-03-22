/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  EvalDiagnostic.hpp
 *  @brief MABE Evaluation module for counting the number of ones (or zeros) in an output.
 */

#ifndef MABE_EVAL_DIAGNOSTIC_H
#define MABE_EVAL_DIAGNOSTIC_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  class EvalDiagnostic : public Module {
  private:
    Collection target_collect;  // Which organisms should we evaluate?

    std::string vals_trait;     // Set of values to evaluate
    std::string scores_trait;   // Vector of scores for each value
    std::string total_trait;    // A single value totalling all of the scores.

    enum Type {
      EXPLOIT,                  // Must drive values as close to 100 as possible.
      STRUCTURED_EXPLOIT,       // Start at first value; only count values smaller than previous.
      EXPLORE,                  // Start at max value; keep counting values if less than previous.
      DIVERSITY,                // ONLY count max value; all others are max - their current value.
      WEAK_DIVERSITY,           // ONLY count max value; all others don't count (and can drift)
      NUM_DIAGNOSTICS,
      UNKNOWN
    };

    Type type;

    // Helper functions.
    std::string TypeToName(Type type) {
      switch (type) {
        case EXPLOIT: return "exploit"; break;
        case STRUCTURED_EXPLOIT: return "struct_exploit"; break;
        case EXPLORE: return "explore"; break;
        case DIVERSITY: return "diversity"; break;
        case WEAK_DIVERSITY: return "weak_diversity"; break;
        default;
      }
      return "unknown";
    }

    Type NameToType(const std::string & name) {
      if (name == "exploit") return Type::EXPLOIT;
      else if (name == "struct_exploit") return Type::STRUCTURED_EXPLOIT;
      else if (name == "explore") return Type::EXPLORE;
      else if (name == "diversity") return Type::DIVERSITY;
      else if (name == "weak_diversity") return Type::WEAK_DIVERSITY;
      return Type::UNKNOWN;
    }

  public:
    EvalDiagnostic(mabe::MABE & control,
                   const std::string & name="EvalDiagnostic",
                   const std::string & desc="Evaluate bitstrings by counting ones (or zeros).",
                   const std::string & _vtrait="vals",
                   const std::string & _strait="scores",
                   const std::string & _ttrait="total")
      : Module(control, name, desc)
      , target_collect(control.GetPopulation(0))
      , vals_trait(_vtrait)
      , scores_trait(_strait)
      , total_trait(_ttrait)
    {
      SetEvaluateMod(true);
    }
    ~EvalDiagnostic() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Which population(s) should we evaluate?");
      LinkVar(vals_trait, "vals_trait", "Which trait stores the values to evaluate?");
      LinkVar(scores_trait, "scores_trait", "Which trait should we store revised scores in?");
      LinkVar(total_trait, "total_trait", "Which trait should we store the total score in?");
      LinkFuns<std::string>(
        [this](){ return TypeToName(type); },
        [this](const std::string & name){ type = NameToType(name); },
        "disagnostic", "Which Diagnostic should we use?"
        "\n\"exploit\": All values must independently optimize to the max."
        "\n\"struct_exploit\": Values must decrease from begining AND optimize."
        "\n\"explore\": Only count max value and decreasing values after it."
        "\n\"diversity\": Only count max value; all others must be low."
        "\n\"weak_diversity\": Only count max value; all others locked at zero."
      );
    }

    void SetupModule() override {
      AddRequiredTrait<emp::BitVector>(bits_trait);
      AddOwnedTrait<double>(fitness_trait, "All-ones fitness value", 0.0);
    }

    void OnUpdate(size_t update) override {
      emp_assert(control.GetNumPopulations() >= 1);

      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_collect( target_collect.GetAlive() );
      for (Organism & org : alive_collect) {        
        // Make sure this organism has its bit sequence ready for us to access.
        org.GenerateOutput();

        // Count the number of ones in the bit sequence.
        const emp::BitVector & bits = org.GetVar<emp::BitVector>(bits_trait);
        double fitness = (double) bits.CountOnes();

        // If we were supposed to count zeros, subtract ones count from total number of bits.
        if (count_type == 0) fitness = bits.size() - fitness;

        // Store the count on the organism in the fitness trait.
        org.SetVar<double>(fitness_trait, fitness);

        if (fitness > max_fitness || !max_org) {
          max_fitness = fitness;
          max_org = &org;
        }
      }

      std::cout << "Max " << fitness_trait << " = " << max_fitness << std::endl;
    }
  };

  MABE_REGISTER_MODULE(EvalDiagnostic, "Evaluate bitstrings by counting ones (or zeros).");
}

#endif
