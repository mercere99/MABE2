/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2022.
 *
 *  @file  EvalTaskAndnot.h
 *  @brief Tests organism output for bitwise ANDNOT operation
 *
 *  A ANDNOT B is equal to A AND (~B), where ~ is bitwise NOT 
 *  Here, however, we check both directions. So we also look for B AND (~A).
 */

#ifndef MABE_EVAL_TASK_ANDNOT_H
#define MABE_EVAL_TASK_ANDNOT_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"
#include "./EvalTaskBase.hpp"

namespace mabe {

  /// \brief Tests organism output for bitwise ANDNOT operation
  class EvalTaskAndnot : public EvalTaskBase<EvalTaskAndnot> {

  public:
    EvalTaskAndnot(mabe::MABE & control,
                  const std::string & name="EvalTaskAndnot",
                  const std::string & desc="Evaluate organism on ANDNOT logic task")
      : EvalTaskBase(control, name, "andnot", 2, desc){;}

    ~EvalTaskAndnot() { }

    /// Check if passed output is equal to input_a ANDNOT input_b or input_b ANDNOT input_a 
    bool CheckTwoArg(const data_t& output, const data_t& input_a, const data_t& input_b){
      return (output == (input_a & ~input_b)) || (output == (input_b & ~input_a));
    }
  };

  MABE_REGISTER_MODULE(EvalTaskAndnot, "Organism-triggered evaluation of ANDNOT operation");

}

#endif
