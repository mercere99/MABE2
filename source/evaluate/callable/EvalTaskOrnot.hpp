/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  EvalTaskOrnot.h
 *  @brief Tests organism output for bitwise ORNOT operation
 *
 *  A ORNOT B is equal to A OR (~B), where ~ is bitwise NOT.
 *  Here, however, we check both directions. So we also look for B OR (~A).
 */

#ifndef MABE_EVAL_TASK_ORNOT_H
#define MABE_EVAL_TASK_ORNOT_H

#include "./EvalTaskBase.hpp"

namespace mabe {

  /// \brief Tests organism output for bitwise ORNOT operation
  class EvalTaskOrnot : public EvalTaskBase<EvalTaskOrnot, 2> {

  public:
    EvalTaskOrnot(mabe::MABE & control,
                  const std::string & name="EvalTaskOrnot",
                  const std::string & desc="Evaluate organism on ORNOT logic task")
      : EvalTaskBase(control, name, "ornot", desc){;}

    ~EvalTaskOrnot() { }

    
    /// Check if the passed output is equal to input_a ORNOT input_b or input_b ORNOT input_a
    bool CheckTwoArg(const data_t& output, const data_t& input_a, const data_t& input_b){
      return (output == (input_a | ~input_b)) || (output == (input_b | ~input_a));
    }
  };

  MABE_REGISTER_MODULE(EvalTaskOrnot, "Organism-triggered evaluation of ORNOT operation");

}

#endif
