/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskEqu.h
 *  @brief Tests organism output for bitwise EQU operation
 *
 * Note that we are not checking for a binary A == B. 
 * We are looking for the *bitwise* equality. If the Nth bit of A is equal to the Nth bit of B, the Nth bit of the output should be a 1 (and otherwise it should be a 0). 
 */

#ifndef MABE_EVAL_TASK_EQU_H
#define MABE_EVAL_TASK_EQU_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"
#include "./EvalTaskBase.hpp"

namespace mabe {

  /// \brief Tests organism output for bitwise EQU operation
  class EvalTaskEqu : public EvalTaskBase<EvalTaskEqu> {

  public:
    EvalTaskEqu(mabe::MABE & control,
                  const std::string & name="EvalTaskEqu",
                  const std::string & desc="Evaluate organism on EQU logic task")
      : EvalTaskBase(control, name, "equ", 2, desc){;}

    ~EvalTaskEqu() { }
    
    /// Check if the passed output is equal to input_a EQU input_b  
    bool CheckTwoArg(const data_t& output, const data_t& input_a, const data_t& input_b){
      return output == ((input_a & input_b) | ~(input_a | input_b));
    }
  };

  MABE_REGISTER_MODULE(EvalTaskEqu, "Organism-triggered evaluation of EQU operation");

}

#endif
