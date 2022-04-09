/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  EmplodeType.hpp
 *  @brief Setup types for use in scripting.
 *  @note Status: BETA
 */

#ifndef EMPLODE_TYPE_HPP
#define EMPLODE_TYPE_HPP

#include "emp/base/assert.hpp"

#include "Symbol_Scope.hpp"
#include "TypeInfo.hpp"

namespace emplode {

  class Emplode;
  class Symbol_Object;

  // Base class for types that we want to be used for scripting.
  class EmplodeType {
  protected:
    emp::Ptr<Symbol_Scope> symbol_ptr = nullptr;

  public:
    /// Setup the TYPE of object in the config.  This is a stub class, but any new class derived from
    /// EmplodeType can create its own version to automatically load in member functions, etc.
    static void InitType(TypeInfo & /*info*/) {
      // If you create a version of this function for your own EmplodeType, this is where you would
      // create member functions.  Note that this is a static function, so just make a version of it
      // in your own class; you are NOT overriding a virtual function.
    }

    /// If you want this type to be made from another EmplodeType on the fly, build a new version
    /// of this static member function to be called.
    template <typename OUT_T, typename IN_T> static OUT_T MakeRValueFrom(IN_T &&) {
      emp_error("Cannot convert provided input to requested RValue", emp::GetTypeID<OUT_T>());
      return *((OUT_T *) nullptr);
    }


    virtual ~EmplodeType() { }

    virtual std::string ToString() const { return "[[__EMPLODE_OBJECT__]]"; }

    // Optional function to override to add configuration options associated with an object.
    virtual void SetupConfig() { };

    // Normally when an EmplodeType is copied, just the scope variables are copied over.
    // Override CopyValue() if more needs to happen.
    virtual bool CopyValue(const EmplodeType &) { return false; }

    Symbol_Scope & AsScope() {
      emp_assert(!symbol_ptr.IsNull());
      return *symbol_ptr.DynamicCast<Symbol_Scope>();
    }
    const Symbol_Scope & AsScope() const {
      emp_assert(!symbol_ptr.IsNull());
      return *symbol_ptr.DynamicCast<const Symbol_Scope>();
    }

    /// Setup an instance of a new EmplodeType object; provide it with its symbol and type information.    
    void Setup(Symbol_Object & in_symbol) {
      symbol_ptr = &in_symbol;

      // Link specialized variable for the derived type.
      SetupConfig();

      // Load in any member function for this object into the object.
      using symbol_ptr_t = emp::Ptr<Symbol>;
      using member_fun_t = std::function<symbol_ptr_t(const emp::vector<symbol_ptr_t> &)>;
      const auto & member_map = symbol_ptr->GetTypeInfoPtr()->GetMemberFunctions();

      // std::cout << "Loading member functions for '" << in_symbol.GetName() << "'; "
      //           << member_map.size() << " found."
      //           << std::endl;

      for (const MemberFunInfo & member_info : member_map) {
        member_fun_t linked_fun = [this, &member_info](const emp::vector<symbol_ptr_t> & args){
          return member_info.fun(*this, args);
        };
        symbol_ptr->AddFunction(member_info.name, linked_fun,
                                member_info.desc, member_info.return_type).SetBuiltin();

        // std::cout << "Adding member function '" << member_info.name << "' to object '"
        //           << symbol_ptr->GetName() << "'." << std::endl;
      }
    }


    // ---==  Configuration Management ==---

    /// Link a variable to a configuration entry - the value will default to the
    /// variables current value, but be updated when configs are loaded.
    template <typename VAR_T>
    Symbol_Linked<VAR_T> & LinkVar(VAR_T & var,
                                        const std::string & name,
                                        const std::string & desc,
                                        bool is_builtin = false) {
      return AsScope().LinkVar<VAR_T>(name, var, desc, is_builtin);
    }

    /// Link a configuration entry to a pair of functions - it automatically calls the set
    /// function when configs are loaded, and the get function when current value is needed.
    template <typename VAR_T>
    Symbol_LinkedFunctions<VAR_T> & LinkFuns(std::function<VAR_T()> get_fun,
                                            std::function<void(const VAR_T &)> set_fun,
                                            const std::string & name,
                                            const std::string & desc,
                                            bool is_builtin = false) {
      return AsScope().LinkFuns<VAR_T>(name, get_fun, set_fun, desc, is_builtin);
    }

    // Helper functions and info.
    template <typename VAR_T>
    struct MenuEntry {
      VAR_T value;
      std::string name;
      std::string desc;

      MenuEntry(VAR_T v, const std::string & n, const std::string & d)
        : value(v), name(n), desc(d) {}
    };

    /// Link a set of menu option to a variable value.
    /// Each option should include three arguments:
    /// The return value, the option name, and the option description.
    template <typename VAR_T, typename... Ts>
    Symbol_LinkedFunctions<std::string> & LinkMenu(VAR_T & var,
                                                  const std::string & name,
                                                  const std::string & desc,
                                                  const Ts &... entries) {
      auto menu = emp::BuildObjVector<MenuEntry<VAR_T>, 3>(entries...);

      // Build the "get" function: take the current value of the menu and return the name.
      std::function<std::string()> get_fun =
        [&var,menu](){
          for (const MenuEntry<VAR_T> & entry : menu) {
            if (var == entry.value) return entry.name;
          }
          return std::string("UNKNOWN");
        };

      // Build the "set" function: take the name of the menu option and update variable..
      std::function<void(std::string)> set_fun =
        [&var,name,menu](const std::string & entry_name){
          for (const MenuEntry<VAR_T> & entry : menu) {
            if (entry_name == entry.name) { var = entry.value; return; }
          };
          // AddError("Trying to set menu '", name, "' to '", entry_name, "'; does not exist.");
        };

      // Update the description to list all of the menu options.
      std::stringstream new_desc;

      // Start with the input description and add the description for each menu option.
      new_desc << desc;
      for (const MenuEntry<VAR_T> & entry : menu) {
        new_desc << "\n " << entry.name << ": " << entry.desc;
      }

      return AsScope().LinkFuns<std::string>(name, get_fun, set_fun, new_desc.str());
    }
  };
}

#endif
