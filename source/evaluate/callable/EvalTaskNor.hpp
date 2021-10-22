/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskNor.h
 *  @brief Tests organism output for NOR operation on a ManualEval trigger
 */

#ifndef MABE_EVAL_TASK_NOR_H
#define MABE_EVAL_TASK_NOR_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"
#include "./EvalTaskBase.hpp"

namespace mabe {

  class EvalTaskNor : public EvalTaskBase {

  public:
    EvalTaskNor(mabe::MABE & control,
                  const std::string & name="EvalTaskNor",
                  const std::string & desc="Evaluate organism on NOR logic task")
      : EvalTaskBase(control, name, "nor", 2, desc){;}

    ~EvalTaskNor() { }
    
    bool CheckTwoArg(data_t& output, data_t& input_a, data_t& input_b){
      return output == ~(input_a | input_b);
    }
  };

  MABE_REGISTER_MODULE(EvalTaskNor, "Organism-triggered evaluation of NOR operation");

}

#endif
