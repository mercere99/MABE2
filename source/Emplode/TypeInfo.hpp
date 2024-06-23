/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2024.
 *
 *  @file  TypeInfo.hpp
 *  @brief Manages all of the information about a particular type in the config language.
 *  @note Status: BETA
 */

#ifndef EMPLODE_TYPE_INFO_HPP
#define EMPLODE_TYPE_INFO_HPP

#include <fstream>

#include "emp/base/assert.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/tools/String.hpp"

#include "Symbol.hpp"
#include "SymbolTableBase.hpp"

namespace emplode {

  // Information about a member function.
  struct MemberFunInfo {
    using symbol_ptr_t = emp::Ptr<Symbol>;
    using fun_t = std::function<symbol_ptr_t(EmplodeType &, const emp::vector<symbol_ptr_t> &)>;

    emp::String name;
    emp::String desc;
    fun_t fun;
    emp::TypeID return_type;

    MemberFunInfo(const emp::String & in_name, const emp::String & in_desc,
                  fun_t in_fun, emp::TypeID in_rtype)
      : name(in_name), desc(in_desc), fun(in_fun), return_type(in_rtype) {}
  };

  // TypeInfo tracks a particular type to be used in the configuration language.
  class TypeInfo {
  private:
    using init_fun_t = std::function<emp::Ptr<EmplodeType> (const emp::String &)>;
    using copy_fun_t = std::function<bool (const EmplodeType &, EmplodeType &)>;

    SymbolTableBase & symbol_table; // Which symbol table are we part of?

    size_t index;
    emp::String type_name;
    emp::String desc;
    emp::TypeID type_id;

    init_fun_t init_fun;
    copy_fun_t copy_fun;
    bool config_owned = false; // Should objects of this type be managed by Emplode?

    emp::vector< MemberFunInfo > member_funs;

  public:
    // Constructor to allow a simple new configuration type
    TypeInfo(SymbolTableBase & _st, size_t _id, const emp::String & _name, const emp::String & _desc)
      : symbol_table(_st), index(_id), type_name(_name), desc(_desc)
    {
      emp_assert(type_name != "");
    }

    // Constructor to allow a new configuration type whose objects require initialization.
    TypeInfo(SymbolTableBase & _st, size_t _id, const emp::String & _name, const emp::String & _desc,
             init_fun_t _init, copy_fun_t _copy, bool _config_owned=false)
      : symbol_table(_st), index(_id), type_name(_name), desc(_desc),
        init_fun(_init), copy_fun(_copy), config_owned(_config_owned)
    {
      emp_assert(type_name != "");
    }

    size_t GetIndex() const { return index; }
    const emp::String & GetTypeName() const { return type_name; }
    const emp::String & GetDesc() const { return desc; }
    emp::TypeID GetTypeID() const { return type_id; }
    bool GetOwned() const { return config_owned; }
    const emp::vector<MemberFunInfo> & GetMemberFunctions() const { return member_funs; }

    emp::Ptr<EmplodeType> MakeObj(const emp::String & name="__temp__") const {
      emp_assert(init_fun, "No initialization function exists for type.", type_name);
      return init_fun(name);
    }
    bool CopyObj(const EmplodeType & from, EmplodeType & to) const {
      if (copy_fun) return copy_fun(from, to);
      return false;
    }

    // Link this TypeInfo object to a real C++ type.
    // @CAO It would be nice to test to make sure this is an EmplodeType, but not possible with a TypeID.
    void LinkType(emp::TypeID in_id) { type_id = in_id; }

    // Add a member function that can be called on objects of this type.
    template <typename FUN_T>
    void AddMemberFunction(
      const emp::String & name,
      FUN_T fun,
      const emp::String & desc
    ) {
      // std::cout << "Adding member function '" << name
      //           << "' to type '" << type_name << "'."
      //           << "  (Entry #" << member_funs.size() << ")"
      //           << std::endl;

      // ----- Transform this function into one that TypeInfo can make use of ----
      MemberFunInfo::fun_t member_fun = symbol_table.WrapMemberFunction(type_id, name, fun);

      // Add this member function to the library we are building.
      using return_t = typename emp::FunInfo<FUN_T>::return_t;
      member_funs.emplace_back(name, desc, member_fun, emp::GetTypeID<return_t>());
    }
  };

}
#endif
