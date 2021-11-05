/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
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
#include "emp/tools/string_utils.hpp"

#include "Symbol.hpp"
#include "EmplodeTools.hpp"

namespace emplode {

  // Information about a member function.
  struct MemberFunInfo {
    using symbol_ptr_t = emp::Ptr<Symbol>;
    using fun_t = std::function<symbol_ptr_t(EmplodeType &, const emp::vector<symbol_ptr_t> &)>;

    std::string name;
    std::string desc;
    fun_t fun;

    MemberFunInfo(const std::string & in_name, const std::string & in_desc, fun_t in_fun)
      : name(in_name), desc(in_desc), fun(in_fun) {}
  };

  // TypeInfo tracks a particular type to be used in the configuration langauge.
  class TypeInfo {
  private:
    using init_fun_t = std::function<emp::Ptr<EmplodeType> (const std::string &)>;
    using copy_fun_t = std::function<bool (const EmplodeType &, EmplodeType &)>;

    size_t index;
    std::string type_name;
    std::string desc;
    emp::TypeID type_id;

    init_fun_t init_fun;
    copy_fun_t copy_fun;
    bool config_owned = false; // Should objects of this type be managed by Emplode?

    emp::vector< MemberFunInfo > member_funs;

  public:
    // Constructor to allow a simple new configuration type
    TypeInfo(size_t _id, const std::string & _name, const std::string & _desc)
      : index(_id), type_name(_name), desc(_desc) { }

    // Constructor to allow a new configuration type whose objects require initialization.
    TypeInfo(size_t _id, const std::string & _name, const std::string & _desc,
             init_fun_t _init, copy_fun_t _copy, bool _config_owned=false)
      : index(_id), type_name(_name), desc(_desc),
        init_fun(_init), copy_fun(_copy), config_owned(_config_owned)
    { }

    size_t GetIndex() const { return index; }
    const std::string & GetTypeName() const { return type_name; }
    const std::string & GetDesc() const { return desc; }
    emp::TypeID GetTypeID() const { return type_id; }
    bool GetOwned() const { return config_owned; }
    const emp::vector<MemberFunInfo> & GetMemberFunctions() const { return member_funs; }

    emp::Ptr<EmplodeType> MakeObj(const std::string & name) const {
      emp_assert(init_fun, "No initialization function exists for type.", type_name);
      return init_fun(name);
    }
    bool CopyObj(const EmplodeType & from, EmplodeType & to) const {
      emp_assert(copy_fun, "No copy function exists for type.", type_name);
      return copy_fun(from, to);
    }

    // Link this TypeInfo object to a real C++ type.
    // @CAO It would be nice to test to make sure this is an EmplodeType, but not possible with a TypeID.
    void LinkType(emp::TypeID in_id) { type_id = in_id; }

    // Add a member function that can be called on objects of this type.
    template <typename FUN_T>
    void AddMemberFunction(
      const std::string & name,
      FUN_T fun,
      const std::string & desc
    ) {
      // std::cout << "Adding member function '" << name
      //           << "' to type '" << type_name << "'."
      //           << "  (Entry #" << member_funs.size() << ")"
      //           << std::endl;

      // ----- Transform this function into one that TypeInfo can make use of ----
      MemberFunInfo::fun_t member_fun = EmplodeTools::WrapMemberFunction(type_id, name, fun);

      // Add this member function to the library we are building.
      member_funs.emplace_back(name, desc, member_fun);
    }
  };

}
#endif
