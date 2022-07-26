/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  EvalTaskNor.h
 *  @brief Tests organism output for bitwise NOR operation
 */

#ifndef MABE_EVAL_TASK_NOR_H
#define MABE_EVAL_TASK_NOR_H

#include "./EvalTaskBase.hpp"

namespace mabe {

  /// \brief Tests organism output for bitwise NOR operation
  class EvalTaskNor : public EvalTaskBase<EvalTaskNor, 2> {

  public:
    EvalTaskNor(mabe::MABE & control,
                  const std::string & name="EvalTaskNor",
                  const std::string & desc="Evaluate organism on NOR logic task")
      : EvalTaskBase(control, name, "nor", desc){;}

    ~EvalTaskNor() { }
    
    /// Check if the passed output is equal to input_a NOR input_b  
    bool CheckTwoArg(const data_t& output, const data_t& input_a, const data_t& input_b){
      return output == ~(input_a | input_b);
    }
  };

  MABE_REGISTER_MODULE(EvalTaskNor, "Organism-triggered evaluation of NOR operation");

}

#endif
