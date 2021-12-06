/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskOrnot.h
 *  @brief Tests organism output for ORNOT operation on a ManualEval trigger
 */

#ifndef MABE_EVAL_TASK_ORNOT_H
#define MABE_EVAL_TASK_ORNOT_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"
#include "./EvalTaskBase.hpp"

namespace mabe {

  class EvalTaskOrnot : public EvalTaskBase {

  public:
    EvalTaskOrnot(mabe::MABE & control,
                  const std::string & name="EvalTaskOrnot",
                  const std::string & desc="Evaluate organism on ORNOT logic task")
      : EvalTaskBase(control, name, "ornot", 2, desc){;}

    ~EvalTaskOrnot() { }

    
    bool CheckTwoArg(const data_t& output, const data_t& input_a, const data_t& input_b){
      return (output == (input_a | ~input_b)) || (output == (input_b | ~input_a));
    }
  };

  MABE_REGISTER_MODULE(EvalTaskOrnot, "Organism-triggered evaluation of ORNOT operation");

}

#endif
