/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  EvalTaskNand.h
 *  @brief Tests organism output for bitwise NAND operation
 */

#ifndef MABE_EVAL_TASK_NAND_H
#define MABE_EVAL_TASK_NAND_H

#include "./EvalTaskBase.hpp"

namespace mabe {

  /// \brief Tests organism output for bitwise NAND operation
  class EvalTaskNand : public EvalTaskBase<EvalTaskNand, 2> {
  public:
    EvalTaskNand(mabe::MABE & control,
                  const std::string & name="EvalTaskNand",
                  const std::string & desc="Evaluate organism on NAND logic task")
      : EvalTaskBase(control, name, "nand", desc){;}

    /// Check if the passed output is equal to input_a NAND input_b  
    bool CheckTwoArg(const data_t& output, const data_t& input_a, const data_t& input_b){
      return output == ~(input_a & input_b);
    }
  };
    
  MABE_REGISTER_MODULE(EvalTaskNand, "Organism-triggered evaluation of NAND operation");

}

#endif
