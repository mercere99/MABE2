/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  AllOnes.cc
 *  @brief Implementation of a simple all-ones problem using MABE.
 */

#include <iostream>

#include "config/ArgManager.h"
#include "tools/BitVector.h"
#include "tools/Random.h"

#include "../source/core/MABE.h"
#include "../source/evaluate/EvalAll1s.h"
#include "../source/orgs/BitsOrg.h"
#include "../source/interface/CommandLine.h"
#include "../source/placement/GrowthPlacement.h"
#include "../source/schema/Mutate.h"
#include "../source/select/SelectElite.h"
#include "../source/select/SelectTournament.h"

int main(int argc, char* argv[])
{
  mabe::MABE control(argc, argv);
  control.AddModule<mabe::CommandLine>();
  control.AddOrganismManager<mabe::BitsOrgManager>("BitOrg");
  control.AddModule<mabe::Mutate>(0, 1);
  control.AddModule<mabe::EvalAll1s>("bits", "fitness");
  control.AddModule<mabe::SelectElite>("fitness", 1, 1);
  control.AddModule<mabe::SelectTournament>("fitness", 7, 199);
  control.AddModule<mabe::GrowthPlacement>();
  control.Setup();
  control.Inject("BitOrg", 200);
  control.Update(100);

/*
  // emp::EAWorld<BitOrg, emp::FitCacheOff> pop(random, "NKWorld");
  emp::World<BitOrg> pop(random, "NKWorld");
  pop.SetupFitnessFile().SetTimingRepeat(10);
  pop.SetupSystematicsFile().SetTimingRepeat(10);
  pop.SetupPopulationFile().SetTimingRepeat(10);
  pop.SetPopStruct_Mixed(true);
  pop.SetCache();

  // Build a random initial population
  for (uint32_t i = 0; i < POP_SIZE; i++) {
    BitOrg next_org(N);
    for (uint32_t j = 0; j < N; j++) next_org[j] = random.P(0.5);
    pop.Inject(next_org);
  }

  // Setup the mutation function.
  std::function<size_t(BitOrg &, emp::Random &)> mut_fun =
    [MUT_COUNT, N](BitOrg & org, emp::Random & random) {
      size_t num_muts = 0;
      for (uint32_t m = 0; m < MUT_COUNT; m++) {
        const uint32_t pos = random.GetUInt(N);
        if (random.P(0.5)) {
          org[pos] ^= 1;
          num_muts++;
        }
      }
      return num_muts;
    };
  pop.SetMutFun( mut_fun );
  pop.SetAutoMutate();

  std::cout << 0 << " : " << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;

  std::function<double(BitOrg&)> fit_fun =
    [&landscape](BitOrg & org){ return landscape.GetFitness(org); };
  pop.SetFitFun( fit_fun );

  // Loop through updates
  for (uint32_t ud = 0; ud < MAX_GENS; ud++) {
    // Print current state.
    // for (uint32_t i = 0; i < pop.GetSize(); i++) std::cout << pop[i] << std::endl;
    // std::cout << std::endl;

    // Keep the best individual.
    emp::EliteSelect(pop, 1, 1);

    // Run a tournament for the rest...
    TournamentSelect(pop, 5, POP_SIZE-1);
    pop.Update();
    std::cout << (ud+1) << " : " << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;
  }

  // pop.PrintLineage(0);

//  std::cout << MAX_GENS << " : " << pop[0] << " : " << landscape.GetFitness(pop[0]) << std::endl;

  // pop.GetSignalControl().PrintNames();
  */
}
