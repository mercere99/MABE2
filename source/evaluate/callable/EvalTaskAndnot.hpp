/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskAndnot.h
 *  @brief Tests organism output for ANDNOT operation on a ManualEval trigger
 */

#ifndef MABE_EVAL_TASK_ANDNOT_H
#define MABE_EVAL_TASK_ANDNOT_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"
#include "./EvalTaskBase.hpp"

namespace mabe {

  class EvalTaskAndnot : public EvalTaskBase {

  public:
    EvalTaskAndnot(mabe::MABE & control,
                  const std::string & name="EvalTaskAndnot",
                  const std::string & desc="Evaluate organism on ANDNOT logic task")
      : EvalTaskBase(control, name, "andnot", 2, desc){;}

    ~EvalTaskAndnot() { }

    bool CheckTwoArg(data_t& output, data_t& input_a, data_t& input_b){
      return (output == (input_a & ~input_b)) || (output == (input_b & ~input_a));
    }
  };

  MABE_REGISTER_MODULE(EvalTaskAndnot, "Organism-triggered evaluation of ANDNOT operation");

}

#endif
