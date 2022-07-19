/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  EvalTaskAnd.h
 *  @brief Tests organism output for bitwise AND operation
 */

#ifndef MABE_EVAL_TASK_AND_H
#define MABE_EVAL_TASK_AND_H

#include "./EvalTaskBase.hpp"

namespace mabe {

  /// \brief Tests organism output for bitwise AND operation
  class EvalTaskAnd : public EvalTaskBase<EvalTaskAnd, 2> {

  public:
    EvalTaskAnd(mabe::MABE & control,
                  const std::string & name="EvalTaskAnd",
                  const std::string & desc="Evaluate organism on AND logic task")
      : EvalTaskBase(control, name, "and", desc){;}

    ~EvalTaskAnd() { }
   
    /// Check if the passed output is equal to input_a AND input_b  
    bool CheckTwoArg(const data_t& output, const data_t& input_a, const data_t& input_b){
      return output == (input_a & input_b);
    }
  };

  MABE_REGISTER_MODULE(EvalTaskAnd, "Organism-triggered evaluation of AND operation");

}

#endif
