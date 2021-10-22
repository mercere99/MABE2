/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskAnd.h
 *  @brief Tests organism output for AND operation on a ManualEval trigger
 */

#ifndef MABE_EVAL_TASK_AND_H
#define MABE_EVAL_TASK_AND_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"
#include "./EvalTaskBase.hpp"

namespace mabe {

  class EvalTaskAnd : public EvalTaskBase {

  public:
    EvalTaskAnd(mabe::MABE & control,
                  const std::string & name="EvalTaskAnd",
                  const std::string & desc="Evaluate organism on AND logic task")
      : EvalTaskBase(control, name, "and", 2, desc){;}

    ~EvalTaskAnd() { }
    
    bool CheckTwoArg(data_t& output, data_t& input_a, data_t& input_b){
      return output == (input_a & input_b);
    }
  };

  MABE_REGISTER_MODULE(EvalTaskAnd, "Organism-triggered evaluation of AND operation");

}

#endif
