/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  modules.hpp
 *  @brief A full set of all standard modules available in MABE.
 */

// Evaluation Modules
#include "evaluate/games/EvalMancala.hpp"
#include "evaluate/static/EvalCountBits.hpp"
#include "evaluate/static/EvalDiagnostic.hpp"
#include "evaluate/static/EvalMatchBits.hpp"
#include "evaluate/static/EvalNK.hpp"
#include "evaluate/static/EvalRoyalRoad.hpp"

// Interface Modules
#include "interface/CommandLine.hpp"
#include "interface/FileOutput.hpp"

// Placement Modules
#include "placement/GrowthPlacement.hpp"

// Selection Modules
#include "select/SelectElite.hpp"
#include "select/SelectTournament.hpp"
#include "select/SelectLexicase.hpp"

// Other schema
#include "schema/MovePopulation.hpp"
#include "schema/Mutate.hpp"

// Organism Types
#include "orgs/AvidaGPOrg.hpp"
#include "orgs/BitsOrg.hpp"
#include "orgs/ValsOrg.hpp"
