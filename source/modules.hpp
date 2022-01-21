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
#include "evaluate/games/EvalPathFollow.hpp"
#include "evaluate/static/EvalCountBits.hpp"
#include "evaluate/static/EvalDiagnostic.hpp"
#include "evaluate/static/EvalMatchBits.hpp"
#include "evaluate/static/EvalNK.hpp"
#include "evaluate/static/EvalRoyalRoad.hpp"
#include "evaluate/callable/EvalTaskNot.hpp"
#include "evaluate/callable/EvalTaskNand.hpp"
#include "evaluate/callable/EvalTaskAnd.hpp"
#include "evaluate/callable/EvalTaskOr.hpp"
#include "evaluate/callable/EvalTaskAndnot.hpp"
#include "evaluate/callable/EvalTaskOrnot.hpp"
#include "evaluate/callable/EvalTaskNor.hpp"
#include "evaluate/callable/EvalTaskXor.hpp"
#include "evaluate/callable/EvalTaskEqu.hpp"
#include "evaluate/static/EvalPacking.hpp"

// Placement Modules
#include "placement/AnnotatePlacement.hpp"
#include "placement/RandomReplacement.hpp"
#include "placement/MaxSizePlacement.hpp"

// Selection Modules
#include "select/SelectElite.hpp"
#include "select/SelectLexicase.hpp"
#include "select/SchedulerProbabilistic.hpp"
#include "select/SelectRoulette.hpp"
#include "select/SelectTournament.hpp"

// Organism Types
#include "orgs/BitsOrg.hpp"
#include "orgs/ValsOrg.hpp"
#include "orgs/AvidaGPOrg.hpp"
#include "orgs/VirtualCPUOrg.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Nop.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Math.hpp"
#include "orgs/instructions/VirtualCPU_Inst_IO.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Flow.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Label.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Manipulation.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Replication.hpp"
