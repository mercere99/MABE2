/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  ManagerModule.hpp
 *  @brief Base module to manage a selection of objects that share a common configuration.
 */

#ifndef MABE_MANAGER_MODULE_H
#define MABE_MANAGER_MODULE_H

#include "emp/meta/TypeID.hpp"

#include "../config/Config.hpp"

#include "MABE.hpp"
#include "Module.hpp"

namespace mabe {

  // Pre-declarations...
  class MABE;
  template <typename MANAGED_T, typename BASE_T> class ManagerModule;

  /// Base class for managed products that uses "curiously recursive templates" to fill
  /// out default functionality for when you know the derived type.
  template <typename MANAGED_T, typename BASE_T>
  class ProductTemplate : public BASE_T {
  public:
    ProductTemplate(ModuleBase & _man) : BASE_T(_man) { ; }

    using managed_t = MANAGED_T;
    using manager_t = ManagerModule<MANAGED_T, BASE_T>;

    /// Get the manager for this type of organism.
    manager_t & GetManager() {
      return (manager_t &) BASE_T::GetManager();
    }
    const manager_t & GetManager() const {
      return (const manager_t &) BASE_T::GetManager();
    }

    auto & SharedData() { return GetManager().data; }
    const auto & SharedData() const { return GetManager().data; }
  };


  /// @param MANAGED_T the type of object type being managed.
  /// @param BASE_T the base type being managed.
  template <typename MANAGED_T, typename BASE_T>
  class ManagerModule : public Module {
    /// Allow managed products to access private shared data in their own manager only.
    friend class ProductTemplate<MANAGED_T, BASE_T>;

  private:
    /// Locate the specification for the data that we need for management in the manager module.
    using data_t = typename MANAGED_T::ManagerData;
    
    /// Shared data across all objects that use the same manager.
    data_t data;

    /// Maintain a prototype for the objects being created.
    emp::Ptr<BASE_T> obj_prototype;

  public:
    ManagerModule(MABE & in_control, const std::string & in_name, const std::string & in_desc="")
      : Module(in_control, in_name, in_desc)
    {
      SetManageMod(); // @CAO should specify what type of object is managed.
      obj_prototype = emp::NewPtr<managed_t>(*this);
    }
    virtual ~ManagerModule() { obj_prototype.Delete(); }

    /// Save the type that uses this manager.
    using managed_t = MANAGED_T;

    /// Also get the TypeID for more run-time type management.
    emp::TypeID GetObjType() const override { return emp::GetTypeID<managed_t>(); }

    /// Create a clone of the provided object; default to using copy constructor.
    emp::Ptr<OrgType> CloneObject_impl(const OrgType & obj) override {
      return emp::NewPtr<managed_t>( (const managed_t &) obj );
    }

    /// Create a random object from scratch.  Default to using the obj_prototype object.
    emp::Ptr<OrgType> Make_impl() override {
      auto obj_ptr = obj_prototype->Clone();
      return obj_ptr;
    }

    /// Create a random object from scratch.  Default to using the obj_prototype object
    /// and then randomize if a random number generator is provided.
    emp::Ptr<OrgType> Make_impl(emp::Random & random) override {
      auto obj_ptr = obj_prototype->Clone();
      obj_ptr->Initialize(random);
      return obj_ptr;
    }


    void SetupModule() override {
      obj_prototype->SetupModule();
    }

    void SetupDataMap(emp::DataMap & in_dm) override {
      obj_prototype->SetDataMap(in_dm);
    }

    void SetupConfig() override {
      obj_prototype->SetupConfig();
    }

  };

  /// Build a class that will automatically register modules when created (globally)
  template <typename MODULE_T>
  struct ManagerModuleRegistrar {
    ManagerModuleRegistrar(const std::string & type_name, const std::string & desc) {
      ModuleInfo new_info;
      new_info.name = type_name;
      new_info.desc = desc;
      new_info.init_fun = [desc](MABE & control, const std::string & name) -> ConfigType & {
        return control.AddModule<MODULE_T>(name, desc);
      };
      GetModuleInfo().insert(new_info);
    }
  };


  /// MACRO for quickly adding new manager modules.
  #define MABE_REGISTER_MANAGER_MODULE(TYPE, BASE_TYPE, DESC) \
        mabe::ManagerModuleRegistrar<ManagerModule<TYPE, BASE_TYPE>> MABE_ ## TYPE ## _Registrar(#TYPE, DESC)

}

#endif
