/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  MABE.cc
 *  @brief Dynamic MABE executable that can be fully configured from a file.
 */

#include <iostream>

// Empirical tools
#include "config/ArgManager.h"
#include "tools/BitVector.h"
#include "tools/Random.h"

// Framework
#include "../source/core/MABE.h"

// Modules
#include "../source/evaluate/EvalAll1s.h"
#include "../source/evaluate/EvalNK.h"
#include "../source/interface/CommandLine.h"
#include "../source/placement/GrowthPlacement.h"
#include "../source/schema/Mutate.h"
#include "../source/select/SelectElite.h"
#include "../source/select/SelectTournament.h"

// Organisms
#include "../source/orgs/BitsOrg.h"

int main(int argc, char* argv[])
{
  mabe::MABE control(argc, argv);
  // control.AddPopulation("main_pop");
  // control.AddPopulation("next_pop");
  // control.AddModule<mabe::CommandLine>("CommandLine");
  // control.AddOrganismManager<mabe::BitsOrgManager>("BitOrg");
  // control.AddModule<mabe::Mutate>("Mutate");
  // control.AddModule<mabe::EvalNK>("EvalNK");
  // control.AddModule<mabe::SelectElite>("SelectElite");
  // control.AddModule<mabe::SelectTournament>("SelectTournament");
  // control.AddModule<mabe::GrowthPlacement>();

  // Setup may decide to stop if no additional processing is needed.
  if (control.Setup() == false) return 0;

  control.Inject("bit_org", 200);
  control.Update(1000);
}
