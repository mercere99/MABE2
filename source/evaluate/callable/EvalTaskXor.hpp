/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskXor.h
 *  @brief Tests organism output for XOR operation on a ManualEval trigger
 */

#ifndef MABE_EVAL_TASK_XOR_H
#define MABE_EVAL_TASK_XOR_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"
#include "./EvalTaskBase.hpp"

namespace mabe {

  class EvalTaskXor : public EvalTaskBase {

  public:
    EvalTaskXor(mabe::MABE & control,
                  const std::string & name="EvalTaskXor",
                  const std::string & desc="Evaluate organism on XOR logic task")
      : EvalTaskBase(control, name, "xor", 2, desc){;}

    ~EvalTaskXor() { }
    
    bool CheckTwoArg(const data_t& output, const data_t& input_a, const data_t& input_b){
      return output == (input_a ^ input_b);
    }
  };

  MABE_REGISTER_MODULE(EvalTaskXor, "Organism-triggered evaluation of XOR operation");

}

#endif
