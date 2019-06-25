/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  OrganismType.h
 *  @brief Details about how a specific type of organism should function.
 */

#ifndef MABE_ORGANISM_TYPE_H
#define MABE_ORGANISM_TYPE_H

#include <functional>
#include <iostream>

#include "base/unordered_map.h"
#include "tools/Random.h"

namespace mabe {

  class Organism;

  class OrgTypeBase {
  private:
    std::string name;     ///< Name used for this type of organisms.
    emp::VarMap var_map;  ///< Map of run-time values associated with this organism type.

  public:
    OrgTypeBase(const std::string & in_name) : name(in_name) { ; }
    virtual ~OrgTypeBase() { ; }

    const std::string & GetName() const { return name; }

    virtual size_t MutateOrg(Organism &, emp::Random &) = 0;
    virtual void PrintOrg(Organism &, std::ostream &) = 0;
  };

  template <typename ORG_T>
  class OrganismType : public OrgTypeBase {
  private:
    /// Function to mutate this type of organism.
    std::function<size_t(ORG_T &, emp::Random & random)> mut_fun;
    std::function<void(ORG_T &, std::ostream &)> print_fun;

  public:
    OrganismType(const std::string & in_name) : OrgTypeBase(in_name) {
      mut_fun = [](ORG_T &, emp::Random &){ return 0; }; // No mutation function setup!
      print_fun = [](ORG_T & org, std::ostream & os){ os << org.ToString(); };
    }

    size_t MutateOrg(Organism & org, emp::Random & random) {
      emp_assert(org.GetType() == this);
      return mut_fun((ORG_T &) org, random);
    }
    void SetMutateFun(std::function<size_t(ORG_T &, emp::Random &)> & in_fun) {
      mut_fun = in_fun;
    }

    size_t PrintOrg(Organism & org, std::ostream & os) {
      emp_assert(org.GetType() == this);
      return print_fun(((ORG_T &) org, os);
    }
    void SetPrintFun(std::function<void(ORG_T &, std::ostream &)> & in_fun) {
      print_fun = in_fun;
    }
  };

}

#endif
