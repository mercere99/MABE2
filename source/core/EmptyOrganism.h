/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  EmptyOrganism.h
 *  @brief Simple organism placeholder to represent open positions in a population.
 */

#ifndef MABE_EMPTY_ORGANISM_H
#define MABE_EMPTY_ORGANISM_H

#include "Organism.h"

namespace mabe {

  /// An EmptyOrganism is used as a placeholder in an empty cell in a population.
  class EmptyOrganism : public Organism {
  public:
    EmptyOrganism() : Organism(nullptr) { ; }
    emp::Ptr<Organism> Clone() const override { emp_assert(false, "Do not clone EmptyOrganism"); return nullptr; }
    std::string ToString() override { return "[empty]"; }
    size_t Mutate(emp::Random &) override { emp_assert(false, "EmptyOrganism cannot Mutate()"); return -1; }
    void Randomize(emp::Random &) override { emp_assert(false, "EmptyOrganism cannot Randomize()"); }
    void GenerateOutput(const std::string &, size_t) override { emp_assert(false, "EmptyOrganism cannot GenerateOutput()"); }
    emp::TypeID GetOutputType(size_t=0) override { return emp::TypeID(); }
    bool IsEmpty() const noexcept override { return true; }
  };

}

#endif
