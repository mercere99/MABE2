/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  SymbolTable.hpp
 *  @brief Manages linking names to associated data in the Emplode language.
 *  @note Status: BETA
 * 
 */

#ifndef EMPLODE_SYMBOL_TABLE_HPP
#define EMPLODE_SYMBOL_TABLE_HPP

#include <map>
#include <string>
#include <type_traits>

#include "emp/base/Ptr.hpp"
#include "emp/datastructs/map_utils.hpp"
#include "emp/io/StreamManager.hpp"
#include "emp/meta/TypeID.hpp"

#include "Events.hpp"
#include "Symbol_Scope.hpp"


namespace emplode {

  class SymbolTable {
  protected:
    Symbol_Scope root_scope;                                      ///< All variables from the root level.
    std::map<std::string, Events> events_map;                     ///< A map of names to event groups.
    std::unordered_map<std::string, emp::Ptr<TypeInfo>> type_map; ///< All types available in the script.
    emp::StreamManager file_map;                                     ///< Track all file streams by name.

  public:
    SymbolTable(const std::string & name)
    : root_scope(name, "Global scope", nullptr) {
      // Initialize the type map.
      type_map["INVALID"] = emp::NewPtr<TypeInfo>( 0, "/*ERROR*/", "Error, Invalid type!" );
      type_map["Void"] = emp::NewPtr<TypeInfo>( 1, "Void", "Non-type variable; no value" );
      type_map["Value"] = emp::NewPtr<TypeInfo>( 2, "Value", "Numeric variable" );
      type_map["String"] = emp::NewPtr<TypeInfo>( 3, "String", "String variable" );
      type_map["Struct"] = emp::NewPtr<TypeInfo>( 4, "Struct", "User-made structure" );

      file_map.SetOutputDefaultFile();  // Stream manager should default to 'file' output.
    }

    ~SymbolTable() {
      // Clean up type information.
      for (auto [name, ptr] : type_map) ptr.Delete();
    }

    Symbol_Scope & GetRootScope() { return root_scope; }
    const Symbol_Scope & GetRootScope() const { return root_scope; }
    emp::StreamManager & GetFileManager() { return file_map; }

    bool HasEvent(const std::string & name) const { return emp::Has(events_map, name); }
    bool HasType(const std::string & name) const { return emp::Has(type_map, name); }

    /// To add a built-in function (at the root level) provide it with a name and description.
    /// As long as the function only requires types known to the config system, it should be
    /// converted properly.  For a variadic function, the provided function must take a
    /// vector of ASTNode pointers, but may return any known type.
    template <typename FUN_T>
    void AddFunction(const std::string & name, FUN_T fun, const std::string & desc) {
      root_scope.AddBuiltinFunction(name, fun, desc);
    }

    /// To add a type, provide the type name (that can be referred to in a script) and a function
    /// that should be called (with the variable name) when an instance of that type is created.
    /// The function must return a reference to the newly created instance.
    template <typename INIT_FUN_T, typename COPY_FUN_T>
    TypeInfo & AddType(
      const std::string & type_name,
      const std::string & desc,
      INIT_FUN_T init_fun,
      COPY_FUN_T copy_fun,
      emp::TypeID type_id,
      bool is_config_owned=false
    ) {
      emp_assert(!emp::Has(type_map, type_name), type_name, "Type already exists!");
      size_t index = type_map.size();
      auto info_ptr = emp::NewPtr<TypeInfo>( index, type_name, desc,
                                             init_fun, copy_fun, is_config_owned );
      info_ptr->LinkType(type_id);
      type_map[type_name] = info_ptr;
      return *type_map[type_name];
    }

    /// If the linked type can be provided as a template parameter, we can also double check that
    /// it is derived from EmplodeType (as it needs to be...)
    template <typename OBJECT_T, typename INIT_FUN_T, typename COPY_FUN_T>
    TypeInfo & AddType(
      const std::string & type_name,
      const std::string & desc,
      INIT_FUN_T init_fun,
      COPY_FUN_T copy_fun,
      bool is_config_owned=false
    ) {
      static_assert(std::is_base_of<EmplodeType, OBJECT_T>(),
                    "Only EmplodeType objects can be used as a custom config type.");
      TypeInfo & info = AddType(type_name, desc, init_fun, copy_fun,
                                emp::GetTypeID<OBJECT_T>(), is_config_owned);
      OBJECT_T::InitType(info);
      return info;
    }

    /// If init_fun and copy_fun are not specified in add type, build our own and assume that we
    /// own the object.
    template <typename OBJECT_T>
    TypeInfo & AddType(const std::string & type_name, const std::string & desc) {
      auto init_fun = [](const std::string & /*name*/){ return emp::NewPtr<OBJECT_T>(); };
      auto copy_fun = EmplodeTools::DefaultCopyFun<OBJECT_T>();
      return AddType<OBJECT_T>(type_name, desc, init_fun, copy_fun, true);
    }

    Symbol_Object &  MakeObjSymbol(
      const std::string & type_name,
      const std::string & var_name,
      Symbol_Scope & scope
    ) {
      // Retrieve the information about the requested type.
      TypeInfo & type_info = *type_map[type_name];
      const std::string & type_desc = type_info.GetDesc();
      const bool is_config_owned = type_info.GetOwned();

      // Use the TypeInfo associated with the provided type name to build an instance.
      emp::Ptr<EmplodeType> new_obj = type_info.MakeObj(var_name);
 
      // Setup a scope for this new type, linking the object to it.
      Symbol_Object & new_obj_symbol =
        scope.AddObject(var_name, type_desc, new_obj, type_info, is_config_owned);

      // Let the new object know about its scope.
      new_obj->Setup(new_obj_symbol);

      return new_obj_symbol;
    }


    /// Create a new type of event that can be used in the scripting language.
    Events & AddEventType(const std::string & name) {
      emp_assert(!HasEvent(name), "Event type already exists!", name);
      return events_map[name];
    }

    /// Add an instance of an event with an action that should be triggered.
    void AddEvent(const std::string & name, emp::Ptr<ASTNode> action,
                  double first=0.0, double repeat=0.0, double max=-1.0) {
      emp_assert(HasEvent(name), name);
      // Debug ("Adding event instance for '", name, "' (", first, ":", repeat, ":", max, ")");
      events_map[name].AddEvent(action, first, repeat, max);
    }

    /// Indicate the an event trigger value has been updated; trigger associated events.
    void UpdateEventValue(const std::string & name, double new_value) {
      emp_assert(HasEvent(name), name);
      // Debug("Uppdating event value '", name, "' to ", new_value);
      events_map[name].UpdateValue(new_value);
    }

    /// Trigger all events of a type (ignoring trigger values)
    void TriggerEvents(const std::string & name) {
      emp_assert(HasEvent(name), name);
      events_map[name].TriggerAll();
    }

    /// Print all of the events to the provided stream.
    void PrintEvents(std::ostream & os) const {
      for (const auto & x : events_map) {
        x.second.Write(x.first, os);
      }
    }

  };

}
#endif
