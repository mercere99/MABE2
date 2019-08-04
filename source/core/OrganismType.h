/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  OrganismType.h
 *  @brief Generic interface for determining how a specific type of organism should function.
 */

#ifndef MABE_ORGANISM_TYPE_H
#define MABE_ORGANISM_TYPE_H

#include <functional>
#include <iostream>

#include "base/unordered_map.h"
#include "data/VarMap.h"
#include "tools/Random.h"

namespace mabe {

  class Organism;

  class OrganismType {
  private:
    std::string name;     ///< Name used for this type of organisms.
    emp::VarMap var_map;  ///< Map of run-time values associated with this organism type.

  public:
    OrganismType(const std::string & in_name) : name(in_name) { ; }
    virtual ~OrganismType() { ; }

    const std::string & GetName() const { return name; }

    // --== Functions to manipulate config variables ==--
    template <typename T>
    OrganismType & AddVar(const std::string & name, const std::string & desc, const T & def_val) {
      var_map.Add<T>(name, def_val);
      return *this;
    }

    template <typename T>
    const T & GetVar(const std::string & name) {
      return var_map.Get<T>(name);
    }

    // --== Functions to manipulate organisms ==--
    /// Create a clone of the provided organism.
    virtual emp::Ptr<Organism> CloneOrganism(const Organism &) const = 0;
    virtual emp::Ptr<Organism> MakeOrganism(emp::Random &) const = 0;
    virtual size_t Mutate(Organism &, emp::Random &) const = 0;
    virtual void Randomize(Organism &, emp::Random &) const = 0;
    virtual std::ostream & Print(Organism &, std::ostream &) const = 0;

    virtual void SetupConfig(ConfigScope & config_scope) {
      (void) config_scope;
    }
  };

}

#endif
