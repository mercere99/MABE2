/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  EvalRandom.h
 *  @brief Evaluator that gives each organism a random score between 0 and a specified maximum
 */

#ifndef MABE_EVAL_RANDOM_H
#define MABE_EVAL_RANDOM_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

namespace mabe {

  /// \brief Evaluator that assigns each organism a random score between 0 and a given maximum
  class EvalRandom : public Module {
  private:
    double max_score;
    std::string output_trait;

  public:
    EvalRandom(mabe::MABE & control,
           const std::string & name="EvalRandom",
           const std::string & desc=
              "Gives each organism a random score between 0 and a specified max",
           double _max_score=1000, const std::string & _trait="fitness")
      : Module(control, name, desc)
      , max_score(_max_score) 
      , output_trait(_trait)
    {
      SetEvaluateMod(true);
    }
    ~EvalRandom() { }
    
    /// Set up the EVAL method to be used in the config file
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
          [](EvalRandom & mod, Collection list) { return mod.Evaluate(list); },
          "Use EvalRandom to evaluate all orgs in an OrgList.");
    }

    /// Set up configuration variables
    void SetupConfig() override {
      LinkVar(max_score, "max_score", "Maximum value for the random scores");
      LinkVar(output_trait,"output_trait","Which trait should we store the random score in?");
    }

    /// Set up traits
    void SetupModule() override {
      AddOwnedTrait<double>(output_trait, "Random fitness value", 0.0);
    }
    
    /// Randomly assign a score to all organisms in the collection
    double Evaluate(const Collection & orgs) {
      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_orgs( orgs.GetAlive() );
      for (Organism & org : alive_orgs) {
        double fitness = control.GetRandom().GetDouble() * max_score;
        org.SetTrait<double>(output_trait, fitness);

        if (fitness > max_fitness || !max_org) {
          max_fitness = fitness;
          max_org = &org;
        }
      }

      return max_fitness;
    }

    /// Convert population to a collection and evaluate all the organisms in it
    double Evaluate(Population & pop) { return Evaluate( Collection(pop) ); }

    /// Convert string to a collection and evaluate all the organisms in it
    double Evaluate(const std::string & in) { return Evaluate( control.ToCollection(in) ); }

  };

  MABE_REGISTER_MODULE(EvalRandom, 
      "Gives each organism a random score between 0 and a specified max");
}

#endif
