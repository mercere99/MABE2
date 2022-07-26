/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  EvalTaskNot.h
 *  @brief Tests organism output for bitwise NOT operation
 */

#ifndef MABE_EVAL_TASK_NOT_H
#define MABE_EVAL_TASK_NOT_H

#include "./EvalTaskBase.hpp"

namespace mabe {

  /// \brief Tests organism output for bitwise NOT operation
  class EvalTaskNot : public EvalTaskBase<EvalTaskNot, 1> {
  public:
    EvalTaskNot(mabe::MABE & control,
                  const std::string & name="EvalTaskNot",
                  const std::string & desc="Evaluate organism on NOT logic task")
      : EvalTaskBase(control, name, "not", desc){;}

    /// Check if the passed output is bitwise NOT of the passed input  
    bool CheckOneArg(const data_t& output, const data_t& input){
      return output == ~input;
    }
  };
    
  MABE_REGISTER_MODULE(EvalTaskNot, "Organism-triggered evaluation of NOT operation");

}

#endif
