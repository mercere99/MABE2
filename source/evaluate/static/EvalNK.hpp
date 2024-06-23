/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2024.
 *
 *  @file  EvalNK.hpp
 *  @brief MABE Evaluation module for NK Landscapes
 */

#ifndef MABE_EVAL_NK_H
#define MABE_EVAL_NK_H

#include "../../core/EvalModule.hpp"
#include "../../tools/NK.hpp"

#include "emp/datastructs/reference_vector.hpp"

namespace mabe {

  class EvalNK : public EvalModule<EvalNK> {
  private:
    // MABE_REQUIRED_TRAIT(bits, emp::BitVector, "Bit-sequence to evaluate.");
    RequiredTrait<emp::BitVector> bits_trait{this, "bits", "Bit-sequence to evaluate."};
    OwnedTrait<double> fitness_trait{this, "fitness", "NK fitness value"};
    // OwnedTrait<emp::vector<double>> gene_fitness{this, "gene_fitness", "Individual gene fitnesses"};

    // ConfigVar<size_t> N {this, "N", 100, "Total number of bits required in sequence"};
    size_t N = 100;
    size_t K = 2;    
    NKLandscape landscape;
    // bool track_gene_fitness = false;

  public:
    EvalNK(mabe::MABE & control,
           emp::String name="EvalNK",
           emp::String desc="Evaluate bitstrings on an NK Fitness Landscape")
      : EvalModule(control, name, desc) { }
    ~EvalNK() { }

    void SetupConfig() override {
      LinkVar(N, "N", "Total number of bits required in sequence");
      LinkVar(K, "K", "Number of bits used in each gene");
      // LinkVar(track_gene_fitness, "track_gene_fitness", "Should we track the fitness contribution of each gene?");
    }

    void SetupModule() override {
      // Setup the fitness landscape.
      landscape.Config(N, K, control.GetRandom());  // Setup the fitness landscape.
    }

    double EvaluateCollection(const Collection & orgs) override {
      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_orgs( orgs.GetAlive() );
      for (Organism & org : alive_orgs) {
        org.GenerateOutput();
        const auto & bits = bits_trait(org);
        if (bits.size() != N) {
          emp::notify::Error("Org returns ", bits.size(), " bits, but ",
                             N, " bits needed for NK landscape.",
                             "\nOrg: ", org.ToString());
        }

        // if (track_gene_fitness) {
        //   gene_fitness(org) = landscape.GetGeneFitnesses(bits);
        // }
        const double fitness = landscape.GetFitness(bits);
        fitness_trait(org) = fitness;

        if (fitness > max_fitness || !max_org) {
          max_fitness = fitness;
          max_org = &org;
        }
      }

      return max_fitness;
    }

    /// Re-randomize all of the entries.
    double Reset() override {
      landscape.Config(N, K, control.GetRandom());
      return 0.0;
    }
  };

  MABE_REGISTER_MODULE(EvalNK, "Evaluate bitstrings on an NK fitness landscape.\nFor more info about NK models, see: https://en.wikipedia.org/wiki/NK_model");
}

#endif
