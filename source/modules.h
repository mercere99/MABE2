/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  modules.h
 *  @brief A full set of all standard modules available in MABE.
 */

// Evaluation Modules
#include "evaluate/EvalCountBits.h"
#include "evaluate/EvalMatchBits.h"
#include "evaluate/EvalNK.h"
#include "evaluate/EvalRandom.h"
#include "evaluate/EvalTaskNot.h"

// Interface Modules
#include "interface/CommandLine.h"

// Placement Modules
#include "placement/GrowthPlacement.h"
#include "placement/RandomReplacement.h"

// Selection Modules
#include "select/SelectElite.h"
#include "select/SelectTournament.h"
#include "select/SchedulerProbabilistic.h"

// Other schema
#include "schema/MovePopulation.h"
#include "schema/Mutate.h"

// Organism Types
#include "orgs/BitsOrg.h"
#include "orgs/AvidaGP.h"
