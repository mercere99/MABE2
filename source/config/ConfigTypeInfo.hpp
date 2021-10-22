/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  ConfigTypeInfo.hpp
 *  @brief Manages all of the information about a particular type in the config language.
 *  @note Status: BETA
 */

#ifndef MABE_CONFIG_TYPE_INFO_H
#define MABE_CONFIG_TYPE_INFO_H

#include <fstream>

#include "emp/base/assert.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/tools/string_utils.hpp"

#include "ConfigEntry.hpp"
#include "ConfigTools.hpp"

namespace mabe {

  // Information about a member function.
  struct MemberFunInfo {
    using entry_ptr_t = emp::Ptr<ConfigEntry>;
    using fun_t = std::function<entry_ptr_t(ConfigType &, const emp::vector<entry_ptr_t> &)>;

    std::string name;
    std::string desc;
    fun_t fun;

    MemberFunInfo(const std::string & in_name, const std::string & in_desc, fun_t in_fun)
      : name(in_name), desc(in_desc), fun(in_fun) {}
  };

  // ConfigTypeInfo tracks a particular type to be used in the configuration langauge.
  class ConfigTypeInfo {
  private:
    using entry_ptr_t = emp::Ptr<ConfigEntry>;
    using init_fun_t = std::function<ConfigType & (const std::string &)>;

    size_t index;
    std::string type_name;
    std::string desc;
    emp::TypeID type_id;

    init_fun_t init_fun;

    emp::vector< MemberFunInfo > member_funs;

  public:
    // Constructor to allow a simple new configuration type
    ConfigTypeInfo(size_t in_id, const std::string & in_name, const std::string & in_desc)
      : index(in_id), type_name(in_name), desc(in_desc) { }

    // Constructor to allow a new configuration type whose objects require initialization.
    ConfigTypeInfo(size_t in_id, const std::string & in_name, const std::string & in_desc, init_fun_t in_init)
      : index(in_id), type_name(in_name), desc(in_desc), init_fun(in_init)
    {
    }

    size_t GetIndex() const { return index; }
    const std::string & GetTypeName() const { return type_name; }
    const std::string & GetDesc() const { return desc; }
    emp::TypeID GetType() const { return type_id; }
    const emp::vector<MemberFunInfo> & GetMemberFunctions() const { return member_funs; }

    ConfigType & MakeObj(const std::string & name) const { return init_fun(name); }

    // Link this ConfigTypeInfo object to a real C++ type.
    template <typename OBJECT_T>
    void LinkType() {
      static_assert(std::is_base_of<ConfigType, OBJECT_T>(),
                    "Only ConfigType objects can be used as a custom config type.");
      type_id = emp::GetTypeID<OBJECT_T>();
    }

    // Add a member function that can be called on objects of this type.
    template <typename FUN_T>
    void AddMemberFunction(
      const std::string & name,
      FUN_T fun,
      const std::string & desc
    ) {
      // ----- Transform this function into one that ConfigTypeInfo can make use of ----
      MemberFunInfo::fun_t member_fun = ConfigTools::WrapMemberFunction(type_id, name, fun);

      // Add this member function to the library we are building.
      member_funs.emplace_back(name, desc, member_fun);
    }
  };

}
#endif
