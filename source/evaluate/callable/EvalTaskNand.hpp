/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskNand.h
 *  @brief Tests organism output for NAND operation on a ManualEval trigger
 */

#ifndef MABE_EVAL_TASK_NAND_H
#define MABE_EVAL_TASK_NAND_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"
#include "./EvalTaskBase.hpp"

namespace mabe {

  class EvalTaskNand : public EvalTaskBase {
  public:
    EvalTaskNand(mabe::MABE & control,
                  const std::string & name="EvalTaskNand",
                  const std::string & desc="Evaluate organism on NAND logic task")
      : EvalTaskBase(control, name, "nand", 2, desc){;}

    bool CheckTwoArg(const data_t& output, const data_t& input_a, const data_t& input_b){
      return output == ~(input_a & input_b);
    }
  };
    
  MABE_REGISTER_MODULE(EvalTaskNand, "Organism-triggered evaluation of NAND operation");

}

#endif
