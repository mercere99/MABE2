/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  EvalTaskXor.h
 *  @brief Tests organism output for bitwise XOR operation
 */

#ifndef MABE_EVAL_TASK_XOR_H
#define MABE_EVAL_TASK_XOR_H

#include "./EvalTaskBase.hpp"

namespace mabe {

  /// \brief Tests organism output for bitwise XOR operation
  class EvalTaskXor : public EvalTaskBase<EvalTaskXor, 2> {

  public:
    EvalTaskXor(mabe::MABE & control,
                  const std::string & name="EvalTaskXor",
                  const std::string & desc="Evaluate organism on XOR logic task")
      : EvalTaskBase(control, name, "xor", desc){;}

    ~EvalTaskXor() { }
    
    /// Check if the passed output is equal to input_a XOR input_b  
    bool CheckTwoArg(const data_t& output, const data_t& input_a, const data_t& input_b){
      return output == (input_a ^ input_b);
    }
  };

  MABE_REGISTER_MODULE(EvalTaskXor, "Organism-triggered evaluation of XOR operation");

}

#endif
