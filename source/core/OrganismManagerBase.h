/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  OrganismManagerBase.h
 *  @brief Base class for tracking a category of organism.
 */

#ifndef MABE_ORGANISM_MANAGER_BASE_H
#define MABE_ORGANISM_MANAGER_BASE_H

#include <functional>
#include <iostream>

#include "base/unordered_map.h"
#include "data/VarMap.h"
#include "tools/Random.h"

#include "../config/Config.h"

namespace mabe {

  class Organism;
  class MABE;

  class OrganismManagerBase  : public mabe::ConfigType {
  protected:
    std::string name;              ///< Name used for this type of organisms.
    emp::VarMap var_map;           ///< Map of run-time values associated with this organism type.

    emp::Ptr<Organism> prototype;  ///< Base organism to copy.

  public:
    OrganismManagerBase(const std::string & in_name) : name(in_name) { ; }
    virtual ~OrganismManagerBase() { ; }

    const std::string & GetName() const { return name; }
    virtual std::string GetTypeName() const { return "OrganismManagerBase (base)"; }

    // --== Functions to manipulate config variables ==--
    template <typename T>
    OrganismManagerBase & AddVar(const std::string & name, const std::string & desc, const T & def_val) {
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
    virtual emp::Ptr<Organism> MakeOrganism() const = 0;
    virtual emp::Ptr<Organism> MakeOrganism(emp::Random &) const = 0;
    virtual std::string ToString(const Organism &) const = 0;
    virtual std::ostream & Print(Organism &, std::ostream &) const = 0;

    virtual size_t Mutate(Organism &, emp::Random &) const {
      emp_assert(false, "Mutate() must be overridden for either Organism or OrganismManagerBase.");
      return 0;
    }
    virtual void Randomize(Organism &, emp::Random &) const {
      emp_assert(false, "Randomize() must be overridden for either Organism or OrganismManagerBase.");
    }

    virtual void SetupConfig() { }
  };

  struct OrgManagerInfo {
    std::string name;
    std::string desc;
    std::function<ConfigType & (MABE &, const std::string &)> init_fun;
    bool operator<(const OrgManagerInfo & in) const { return name < in.name; }
  };

  static std::set<OrgManagerInfo> & GetOrgManagerInfo() {
    static std::set<OrgManagerInfo> om_type_info;
    return om_type_info;
  }

  static void PrintOrgManagerInfo() {
    auto & om_info = GetOrgManagerInfo();
    for (auto & om : om_info) {
      std::cout << om.name << " : " << om.desc << std::endl;
    }
  }

}

#endif
