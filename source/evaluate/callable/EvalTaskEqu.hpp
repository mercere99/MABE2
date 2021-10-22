/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskEqu.h
 *  @brief Tests organism output for EQU operation on a ManualEval trigger
 */

#ifndef MABE_EVAL_TASK_EQU_H
#define MABE_EVAL_TASK_EQU_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"
#include "./EvalTaskBase.hpp"

namespace mabe {

  class EvalTaskEqu : public EvalTaskBase {

  public:
    EvalTaskEqu(mabe::MABE & control,
                  const std::string & name="EvalTaskEqu",
                  const std::string & desc="Evaluate organism on EQU logic task")
      : EvalTaskBase(control, name, "equ", 2, desc){;}

    ~EvalTaskEqu() { }
    
    bool CheckTwoArg(data_t& output, data_t& input_a, data_t& input_b){
      return output == ((input_a & input_b) | ~(input_a | input_b));
    }
  };

  MABE_REGISTER_MODULE(EvalTaskEqu, "Organism-triggered evaluation of EQU operation");

}

#endif
