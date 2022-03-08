/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  Symbol_Object.hpp
 *  @brief Extension of scope when there is an external object associated with the structure.
 *  @note Status: BETA
 */

#ifndef EMPLODE_SYMBOL_OBJECT_HPP
#define EMPLODE_SYMBOL_OBJECT_HPP

#include "emp/base/error.hpp"
#include "emp/base/map.hpp"

#include "EmplodeType.hpp"
#include "Symbol_Scope.hpp"

namespace emplode {

  // Set of multiple config entries.
  class Symbol_Object : public Symbol_Scope {
  protected:
    ///< Point to associated object and track ownership
    emp::Ptr<EmplodeType> obj_ptr = nullptr;
    emp::Ptr<TypeInfo> type_info_ptr = nullptr;
    bool obj_owned = false;

  public:
    Symbol_Object(const std::string & _name,
                  const std::string & _desc,
                  emp::Ptr<Symbol_Scope> _scope,
                  emp::Ptr<EmplodeType> _obj,
                  TypeInfo & _type_info,
                  bool _owned)
      : Symbol_Scope(_name, _desc, _scope)
      , obj_ptr(_obj), type_info_ptr(&_type_info), obj_owned(_owned) { }
      
    Symbol_Object(const Symbol_Object & in) = delete;
    Symbol_Object(Symbol_Object && in)
      : Symbol_Scope(std::move(in)), obj_ptr(in.obj_ptr), obj_owned(in.obj_owned)
    {
      // Remove the object from the incoming symbol.
      in.obj_ptr = nullptr;    
      in.obj_owned = false;
    }

    ~Symbol_Object() {
      // If this scope owns its object pointer, delete it now.
      if (obj_owned) obj_ptr.Delete();
    }

    emp::Ptr<EmplodeType> GetObjectPtr() override { return obj_ptr; }  
    emp::Ptr<const EmplodeType> GetObjectPtr() const override { return obj_ptr; }  
    emp::Ptr<const TypeInfo> GetTypeInfoPtr() const override { return type_info_ptr; }

    std::string GetTypename() const override {
      return emp::to_string("[Symbol_Object:", GetObjectType(), "]");
    }

    bool IsObject() const override { return true; }
    emp::TypeID GetObjectType() const override {
      if (type_info_ptr.IsNull()) return emp::GetTypeID<void>();
      return type_info_ptr->GetTypeID();
    }

    /// Set this symbol to be a correctly-typed scope pointer.
    emp::Ptr<Symbol_Object> AsObjectPtr() override { return this; }
    emp::Ptr<const Symbol_Object> AsObjectPtr() const override { return this; }

    bool CopyValue(const Symbol & in) override {
      if (in.IsObject() == false) {
        std::cerr << "Trying to assign `" << in.GetName() << "' to '" << GetName()
                  << "', but " << in.GetName() << " is not an Object." << std::endl;
        std::cerr << "Target: " << in.DebugString() << std::endl;
        emp_error("Assignment failed.");
        return false;   // Mis-matched types; failed to copy.
      }

      // Copy the underlying scope...
      Symbol_Scope::CopyValue(in);

      // Now copy special details for the object.
      const Symbol_Object & in_object = in.AsObject();

      // If typeinfo knows how to make this copy, let it.
      if (type_info_ptr->CopyObj(*in_object.obj_ptr, *obj_ptr)) return true;

      // Otherwise use the default copy method for the object.
      return obj_ptr->CopyValue(*in_object.obj_ptr);
    }

    /// Make a copy of this scope and all of the entries inside it.
    emp::Ptr<Symbol> Clone() const override {
      emp::Ptr<EmplodeType> out_obj = type_info_ptr->MakeObj(); // Create an initial object.
      type_info_ptr->CopyObj(*obj_ptr, *out_obj);               // Copy this object.
      emp::Ptr<Symbol_Scope> out_scope = nullptr;               // Unknown scope?

      // Construct a unique name for the new object.
      std::string out_name = emp::to_string(GetName(), "__", (size_t) out_obj);

      // Build the new Symbol_Object.
      auto out = emp::NewPtr<Symbol_Object>(out_name, GetDesc(), out_scope, out_obj,
                                            *type_info_ptr, obj_owned);

      // Copy over all of the internal symbols.
      for (auto [name, ptr] : symbol_table) { out->symbol_table[name] = ptr->Clone(); }
      // @CAO: Will linkages be in place?

      return out;
    }
  };

  // Definition needed to add an object to an existing scope.
  Symbol_Object & Symbol_Scope::AddObject(
    const std::string & name,
    const std::string & desc,
    emp::Ptr<EmplodeType> obj_ptr,
    TypeInfo & type_info,
    bool obj_owned
  ) {
    return Add<Symbol_Object>(name, desc, this, obj_ptr, type_info, obj_owned);
  }

}
#endif
