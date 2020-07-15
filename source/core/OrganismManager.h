/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  OrganismManager.h
 *  @brief Class to track a category of organism.
 * 
 *  @todo OrganismManagers should be derived from Modules.
 */

#ifndef MABE_ORGANISM_MANAGER_H
#define MABE_ORGANISM_MANAGER_H

#include "meta/TypeID.h"

#include "../config/Config.h"

#include "MABE.h"
#include "Module.h"

namespace mabe {

  class Organism;
  class MABE;

  template <typename ORG_T>
  class OrganismManager  : public Module {
    friend OrganismTemplate<ORG_T>;
  private:
    typename ORG_T::ManagerData data;

  public:
    using org_t = ORG_T;

    OrganismManager(MABE & in_control, const std::string & in_name, const std::string & in_desc="")
      : Module(in_control, in_name, in_desc)
    {
      org_prototype = emp::NewPtr<org_t>(*this);
    }
    virtual ~OrganismManager() { org_prototype.Delete(); }

    emp::TypeID GetOrgType() const override { return emp::GetTypeID<ORG_T>(); }

    /// Convert this organism to the correct type (after ensuring that it is!)
    org_t & ConvertOrg(Organism & org) const {
      emp_assert(&(org.GetManager()) == this);
      return (org_t &) org;
    }

    /// Convert this CONST organism to the correct type (after ensuring that it is!)
    const org_t & ConvertOrg(const Organism & org) const {
      emp_assert(&(org.GetManager()) == this);
      return (const org_t &) org;
    }

    /// Create a clone of the provided organism; default to using copy constructor.
    emp::Ptr<Organism> CloneOrganism(const Organism & org) override {
      return emp::NewPtr<org_t>( ConvertOrg(org) );
    }

    /// Create a random organism from scratch.  Default to using the org_prototype organism.
    emp::Ptr<Organism> MakeOrganism() override {
      auto org_ptr = org_prototype->Clone();
      return org_ptr;
    }

    /// Create a random organism from scratch.  Default to using the org_prototype organism
    /// and then randomize if a random number generator is provided.
    emp::Ptr<Organism> MakeOrganism(emp::Random & random) override {
      auto org_ptr = org_prototype->Clone();
      org_ptr->Randomize(random);
      return org_ptr;
    }

    /// Convert an organism to a string for printing; if not overridden, just prints
    /// "__unknown__".
    std::string OrgToString(const Organism &) const override { return "__unknown__"; };

    /// By default print an organism by triggering it's ToString() function.
    std::ostream & PrintOrganism(Organism & org, std::ostream & os) const override {
      emp_assert(&(org.GetManager()) == this);
      os << org.ToString();
      return os;
    }

    void SetupModule() override {
      org_prototype->SetupModule();
    }

    void SetupDataMap(emp::DataMap & in_dm) override {
      org_prototype->SetDataMap(in_dm);
    }

    void SetupConfig() override {
      org_prototype->SetupConfig();
    }

  };

  /// Build a class that will automatically register modules when created (globally)
  template <typename T>
  struct OrgManagerRegistrar {
    OrgManagerRegistrar(const std::string & type_name, const std::string & desc) {
      ModuleInfo new_info;
      new_info.name = type_name;
      new_info.desc = desc;
      new_info.init_fun = [desc](MABE & control, const std::string & name) -> ConfigType & {
        return control.AddModule<T>(name, desc);
      };
      GetModuleInfo().insert(new_info);
    }
  };

#define MABE_REGISTER_ORG_TYPE(TYPE, DESC) \
        mabe::OrgManagerRegistrar<OrganismManager<TYPE>> MABE_ ## TYPE ## _Registrar(#TYPE, DESC)

}

#endif
