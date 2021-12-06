/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskNot.h
 *  @brief Tests organism output for NOT operation on a ManualEval trigger
 */

#ifndef MABE_EVAL_TASK_NOT_H
#define MABE_EVAL_TASK_NOT_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"
#include "./EvalTaskBase.hpp"

namespace mabe {

  class EvalTaskNot : public EvalTaskBase {
  public:
    EvalTaskNot(mabe::MABE & control,
                  const std::string & name="EvalTaskNot",
                  const std::string & desc="Evaluate organism on NOT logic task")
      : EvalTaskBase(control, name, "not", 1, desc){;}

    bool CheckOneArg(const data_t& output, const data_t& input){
      return output == ~input;
    }
  };
    
  MABE_REGISTER_MODULE(EvalTaskNot, "Organism-triggered evaluation of NOT operation");

}

#endif
