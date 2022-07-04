/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  EvalTaskOr.h
 *  @brief Tests organism output for bitwise OR operation
 */

#ifndef MABE_EVAL_TASK_OR_H
#define MABE_EVAL_TASK_OR_H

#include "./EvalTaskBase.hpp"

namespace mabe {

  /// \brief Tests organism output for bitwise OR operation
  class EvalTaskOr : public EvalTaskBase<EvalTaskOr, 2> {

  public:
    EvalTaskOr(mabe::MABE & control,
                  const std::string & name="EvalTaskOr",
                  const std::string & desc="Evaluate organism on OR logic task")
      : EvalTaskBase(control, name, "or", desc){;}

    ~EvalTaskOr() { }

    /// Check if the passed output is equal to input_a OR input_b  
    bool CheckTwoArg(const data_t& output, const data_t& input_a, const data_t& input_b){
      return output == (input_a | input_b);
    }
  };

  MABE_REGISTER_MODULE(EvalTaskOr, "Organism-triggered evaluation of OR operation");

}

#endif
