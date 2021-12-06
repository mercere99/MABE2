/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskOr.h
 *  @brief Tests organism output for OR operation on a ManualEval trigger
 */

#ifndef MABE_EVAL_TASK_OR_H
#define MABE_EVAL_TASK_OR_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"
#include "./EvalTaskBase.hpp"

namespace mabe {

  class EvalTaskOr : public EvalTaskBase {

  public:
    EvalTaskOr(mabe::MABE & control,
                  const std::string & name="EvalTaskOr",
                  const std::string & desc="Evaluate organism on OR logic task")
      : EvalTaskBase(control, name, "or", 2, desc){;}

    ~EvalTaskOr() { }

    bool CheckTwoArg(const data_t& output, const data_t& input_a, const data_t& input_b){
      return output == (input_a | input_b);
    }
  };

  MABE_REGISTER_MODULE(EvalTaskOr, "Organism-triggered evaluation of OR operation");

}

#endif
