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

#include "Symbol_Object.hpp"
#include "TypeInfo.hpp"

namespace emplode {

  class Emplode;

  // Base class for types that we want to be used for scripting.
  class EmplodeType {
  protected:
    emp::Ptr<Symbol_Object> symbol_ptr;
    emp::Ptr<TypeInfo> type_info_ptr;

    // Some special, internal variables associated with each object.
    bool _active=true;       ///< Should this object be used in the current run?
    std::string _desc="";    ///< Special description for this object.

  public:
    /// Setup the TYPE of object in the config.  This is a stub class, but any new class derived from
    /// EmplodeType can create its own version to automatically load in member functions, etc.
    static void InitType(TypeInfo & /*info*/) {
      // If you create a version of this function for your own EmplodeType, this is where you would
      // create member functions.  Note that this is a static function, so just make a version of it
      // in your own class; you are NOT overriding a virtual function.
    }

    virtual ~EmplodeType() { }

    // Optional function to override to add configuration options associated with an object.
    virtual void SetupConfig() { };

    Symbol_Object & GetScope() { emp_assert(!symbol_ptr.IsNull()); return *symbol_ptr; }
    const Symbol_Object & GetScope() const { emp_assert(!symbol_ptr.IsNull()); return *symbol_ptr; }

    const TypeInfo & GetTypeInfo() const { return *type_info_ptr; }

    /// Setup an instance of a new EmplodeType object; provide it with its symbol and type information.    
    void Setup(Symbol_Object & in_symbol, TypeInfo & _info) {
      symbol_ptr = &in_symbol;
      type_info_ptr = &_info;

      // Link standard internal variables for this object.
      LinkVar(_active, "_active", "Should we activate this module? (0=off, 1=on)", true);
      LinkVar(_desc,   "_desc",   "Special description for those object.", true);

      // Link specialized variable for the derived type.
      SetupConfig();

      // Load in any member function for this object into the object.
      using symbol_ptr_t = emp::Ptr<Symbol>;
      using member_fun_t = std::function<symbol_ptr_t(const emp::vector<symbol_ptr_t> &)>;
      const auto & member_map = type_info_ptr->GetMemberFunctions();

      // std::cout << "Loading member functions for '" << in_symbol.GetName() << "'; "
      //           << member_map.size() << " found."
      //           << std::endl;

      for (const MemberFunInfo & member_info : member_map) {
        member_fun_t linked_fun = [this, &member_info](const emp::vector<symbol_ptr_t> & args){
          return member_info.fun(*this, args);
        };
        symbol_ptr->AddFunction(member_info.name, linked_fun, member_info.desc).SetBuiltin();

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
      return GetScope().LinkVar<VAR_T>(name, var, desc, is_builtin);
    }

    /// Link a configuration entry to a pair of functions - it automatically calls the set
    /// function when configs are loaded, and the get function when current value is needed.
    template <typename VAR_T>
    Symbol_LinkedFunctions<VAR_T> & LinkFuns(std::function<VAR_T()> get_fun,
                                            std::function<void(const VAR_T &)> set_fun,
                                            const std::string & name,
                                            const std::string & desc,
                                            bool is_builtin = false) {
      return GetScope().LinkFuns<VAR_T>(name, get_fun, set_fun, desc, is_builtin);
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

      return GetScope().LinkFuns<std::string>(name, get_fun, set_fun, new_desc.str());
    }
  };
}

#endif
