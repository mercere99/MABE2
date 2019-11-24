/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  OrganismManager.h
 *  @brief Class to track a category of organism.
 */

#ifndef MABE_ORGANISM_MANAGER_H
#define MABE_ORGANISM_MANAGER_H

#include "../config/Config.h"

#include "MABE.h"
#include "OrganismManagerBase.h"

namespace mabe {

  class Organism;
  class MABE;

  template <typename ORG_T>
  class OrganismManager  : public OrganismManagerBase {
  public:
    using org_t = ORG_T;

    OrganismManager(const std::string & in_name) : OrganismManagerBase(in_name) {
      prototype = emp::NewPtr<org_t>(*this);
    }
    virtual ~OrganismManager() { prototype.Delete(); }
  };

  /// Build a class that will automatically register modules when created (globally)
  template <typename T>
  struct OrgManagerRegistrar {
    OrgManagerRegistrar(const std::string & type_name, const std::string & desc) {
      OrgManagerInfo new_info;
      new_info.name = type_name;
      new_info.desc = desc;
      new_info.init_fun = [desc](MABE & control, const std::string & name) -> ConfigType & {
        return control.AddOrganismManager<T>(name, desc);
      };
      GetOrgManagerInfo().insert(new_info);
    }
  };

#define MABE_REGISTER_ORG_MANAGER(TYPE, DESC) \
        mabe::OrgManagerRegistrar<TYPE> MABE_ ## TYPE ## _Registrar(#TYPE, DESC)

}

#endif
