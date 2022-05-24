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

#include "EventManager.hpp"
#include "Symbol_Scope.hpp"
#include "SymbolTableBase.hpp"


namespace emplode {

  class SymbolTable : public SymbolTableBase {
  protected:
    Symbol_Scope root_scope;                                        ///< Outermost (global) scope.
    EventManager event_manager;                                     ///< Event setup & tracking
    std::unordered_map<std::string, emp::Ptr<TypeInfo>> type_map;   ///< Types, lookup by name.
    std::unordered_map<emp::TypeID, emp::Ptr<TypeInfo>> typeid_map; ///< Types, lookup by TypeID.
    emp::StreamManager file_map;                                    ///< File streams by name.

  public:
    SymbolTable(const std::string & name)
    : root_scope(name, "Global scope", nullptr), event_manager(*this) {
      // Initialize the type map.
      type_map["INVALID"] = emp::NewPtr<TypeInfo>( *this, 0, "/*ERROR*/", "Error, Invalid type!" );
      type_map["Void"] = emp::NewPtr<TypeInfo>( *this, 1, "Void", "Non-type variable; no value" );
      type_map["Var"] = emp::NewPtr<TypeInfo>( *this, 2, "Var", "Numeric or String variable" );
      type_map["Struct"] = emp::NewPtr<TypeInfo>( *this, 3, "Struct", "User-made structure" );

      // Those types 
      typeid_map[emp::GetTypeID<void>()] = type_map["Void"];
      typeid_map[emp::GetTypeID<double>()] = type_map["Var"];
      typeid_map[emp::GetTypeID<std::string>()] = type_map["Var"];      

      file_map.SetOutputDefaultFile();  // Stream manager should default to 'file' output.
    }

    ~SymbolTable() {
      // Clean up type information.
      for (auto [name, ptr] : type_map) ptr.Delete();
    }

    Symbol_Scope & GetRootScope() { return root_scope; }
    const Symbol_Scope & GetRootScope() const { return root_scope; }
    emp::StreamManager & GetFileManager() { return file_map; }

    bool HasSignal(const std::string & name) const { return event_manager.HasSignal(name); }
    bool HasType(const std::string & name) const { return emp::Has(type_map, name); }
    bool HasTypeID(emp::TypeID id) const { return emp::Has(typeid_map, id); }

    TypeInfo & GetType(const std::string & type_name) {
      auto type_it = type_map.find(type_name);
      emp_assert(type_it != type_map.end(), "Type name not found in symbol table.", type_name);
      return *(type_it->second);
    }

    const TypeInfo & GetType(const std::string & type_name) const {
      auto type_it = type_map.find(type_name);
      emp_assert(type_it != type_map.end(), "Type name not found in symbol table.", type_name);
      return *(type_it->second);
    }

    /// To add a built-in function (at the root level) provide it with a name and description.
    /// As long as the function only requires types known to the config system, it should be
    /// converted properly.  For a variadic function, the provided function must take a
    /// vector of ASTNode pointers, but may return any known type.
    template <typename FUN_T>
    void AddFunction(const std::string & name, FUN_T fun, const std::string & desc) {
      auto emplode_fun = WrapFunction(name, fun);
      using return_t = typename emp::FunInfo<FUN_T>::return_t;
      emp::TypeID return_id = emp::GetTypeID<return_t>();
      root_scope.AddBuiltinFunction(name, emplode_fun, desc, return_id);
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
      auto info_ptr = emp::NewPtr<TypeInfo>( *this, index, type_name, desc,
                                             init_fun, copy_fun, is_config_owned );
      info_ptr->LinkType(type_id);
      type_map[type_name] = info_ptr;
      typeid_map[type_id] = info_ptr;

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
      auto copy_fun = DefaultCopyFun<OBJECT_T>();
      return AddType<OBJECT_T>(type_name, desc, init_fun, copy_fun, true);
    }

    /// Make a new Symbol_Object using the provided *TypeInfo*, variable name, and scope.
    Symbol_Object & MakeObjSymbol(
      TypeInfo & type_info,
      const std::string & var_name,
      Symbol_Scope & scope
    ) {
      // Retrieve the information about the requested type.
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

    /// Make a new Symbol_Object using the provided type NAME, variable name, and scope.
    Symbol_Object & MakeObjSymbol(const std::string & type_name, const std::string & var_name,
                                   Symbol_Scope & scope) {
      return MakeObjSymbol(*type_map[type_name], var_name, scope);
    }

    /// Make a new Symbol_Object using the provided *TypeID*, variable name, and scope.
    Symbol_Object & MakeObjSymbol(emp::TypeID type_id, const std::string & var_name,
                                   Symbol_Scope & scope) {
      return MakeObjSymbol(*typeid_map[type_id], var_name, scope);
    }


    emp::Ptr<Symbol_Object> MakeTempObjSymbol(emp::TypeID type_id,
                                              emp::Ptr<EmplodeType> value_ptr=nullptr) override {
      TypeInfo & type_info = *typeid_map[type_id];
      emp_assert(type_info.GetOwned(),
                 "Only symbol-owned types can be temporary since they are deleted dynamically.",
                 type_info.GetTypeName());

      // Use the TypeInfo associated with the provided type name to build an instance.
      emp::Ptr<EmplodeType> new_obj = type_info.MakeObj("__Temp");
      auto new_symbol = emp::NewPtr<Symbol_Object>("__Temp", "", nullptr, new_obj, type_info, true);

      new_symbol->SetTemporary();                              // Mark new symbol to be deleted.
      new_obj->Setup(*new_symbol);                             // Setup new object with its symbol.
      if (value_ptr) type_info.CopyObj(*value_ptr, *new_obj);  // Copy value in, if we have one.

      return new_symbol;
    }


    /// Create a new type of event that can be used in the scripting language.
    bool AddSignal(const std::string & name, size_t num_params=0) {
      return event_manager.AddSignal(name, num_params);
    }

    /// Add an instance of an event with an action that should be triggered.
    bool AddAction(
      const std::string & name,
      emp::vector< emp::Ptr<ASTNode> > params,
      emp::Ptr<ASTNode_Block> action,
      size_t def_line
    ) {
      action->SetSymbolTable(*this);
      return event_manager.AddAction(name, params, action, def_line);
    }

    /// Trigger all events of a type (ignoring trigger values)
    template <typename... ARG_Ts>
    bool Trigger(const std::string & signal_name, ARG_Ts... args) {
      return event_manager.Trigger(signal_name, std::forward<ARG_Ts>(args)...);
    }

    /// Print all of the events to the provided stream.
    void PrintEvents(std::ostream & os) const { event_manager.Write(os); }

  };

}
#endif
