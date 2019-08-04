/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  OrganismWrapper.h
 *  @brief Derived OrganismType with details about how a specific type of organism should function.
 */

#ifndef MABE_ORGANISM_WRAPPER_H
#define MABE_ORGANISM_WRAPPER_H

#include <functional>
#include <iostream>

#include "base/unordered_map.h"
#include "data/VarMap.h"
#include "tools/Random.h"

#include "Organism.h"
#include "OrganismType.h"

namespace mabe {

  /// An single type of organism that can have many of its qualities manipulated (and will modify
  /// all organisms of this type.)
  /// NOTE: ORG_T must be derived from mabe::Organism.  When we update to C++20, we can enforce
  ///       this requirement using concepts.
  template <typename ORG_T>
  class OrganismWrapper : public OrganismType {
  private:
    // --== Current versions of user-controled functions to manipulate organisms ==--
    std::function<emp::Ptr<Organism>(emp::Random & random)> make_org_fun;
    std::function<size_t(ORG_T &, emp::Random & random)> mut_fun;
    std::function<std::ostream & (ORG_T &, std::ostream &)> print_fun;
    std::function<bool(ORG_T &, emp::Random & random)> randomize_fun;

  public:
    OrganismWrapper(const std::string & in_name) : OrganismType(in_name) {
      /// --== Initial versions of user-defined functions ==--
      make_org_fun = [](emp::Random & random){
                       auto org = emp::NewPtr<ORG_T>();
                       org->Randomize(random);
                       return org;
                     };
      mut_fun = [](ORG_T & org, emp::Random & random){ return org.Mutate(random); };
      print_fun = [](ORG_T & org, std::ostream & os) -> std::ostream & { os << org.ToString(); return os; };
      randomize_fun = [](ORG_T & org, emp::Random & random){ return org.Randomize(random); };
    }

    emp::Ptr<Organism> MakeOrganism(emp::Random & random) override {
      return make_org_fun(random);
    }
    void SetMakeOrgFun(std::function<emp::Ptr<Organism>(emp::Random &)> & in_fun) {
      make_org_fun = in_fun;
    }

    size_t Mutate(Organism & org, emp::Random & random) override {
      emp_assert(&(org.GetType()) == this);
      return mut_fun((ORG_T &) org, random);
    }
    void SetMutateFun(std::function<size_t(ORG_T &, emp::Random &)> & in_fun) {
      mut_fun = in_fun;
    }

    std::ostream & Print(Organism & org, std::ostream & os) override {
      emp_assert(&(org.GetType()) == this);
      print_fun((ORG_T &) org, os);
      return os;
    }
    void SetPrintFun(std::function<void(ORG_T &, std::ostream &)> & in_fun) {
      print_fun = in_fun;
    }

    bool Randomize(Organism & org, emp::Random & random) override {
      emp_assert(&(org.GetType()) == this);
      return randomize_fun((ORG_T &) org, random);
    }
    void SetRandomizeFun(std::function<size_t(ORG_T &, emp::Random &)> & in_fun) {
      mut_fun = in_fun;
    }

  };

}

#endif