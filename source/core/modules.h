/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  modules.h
 *  @brief A full set of all standard modules available in MABE.
 */

// Evaluation Modules
#include "../source/evaluate/EvalAll1s.h"
#include "../source/evaluate/EvalNK.h"

// Interface Modules
#include "../source/interface/CommandLine.h"

// Placement Modules
#include "../source/placement/GrowthPlacement.h"

// Selection Modules
#include "../source/select/SelectElite.h"
#include "../source/select/SelectTournament.h"

// Other schema
#include "../source/schema/Mutate.h"

// Organism Types
#include "../source/orgs/BitsOrg.h"