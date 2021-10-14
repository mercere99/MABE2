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

class ConfigEntry;

namespace mabe {

  // Information about a member function.
  struct MemberFunInfo {
    using entry_ptr_t = emp::Ptr<ConfigEntry>;
    using fun_t = std::function<entry_ptr_t(ConfigType &, const emp::vector<entry_ptr_t> &)>;

    std::string name;
    std::string desc;
    fun_t fun;
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
    template <typename RETURN_T, typename OBJECT_T, typename... PARAM_Ts>
    void AddMemberFunction(
      const std::string & name,
      std::function<RETURN_T(OBJECT_T &, PARAM_Ts...)> fun,
      const std::string & desc
    ) {
      // ----- Make sure function is legal -----
      // Is return type legal?
      static_assert(std::is_arithmetic<RETURN_T>() ||
                    std::is_same<RETURN_T, std::string>() ||
                    std::is_same<RETURN_T, emp::Ptr<mabe::ConfigEntry>>(),
                    "Config member function return types must be string, arithmetic, or Ptr<ConfigEntry>");

      // Is the first parameter the correct type?
      static_assert(std::is_base_of<ConfigType, typename std::decay<RETURN_T>::type>(),
                    "Member functions must take a reference to the associated ConfigType");
      emp_assert( type_id.IsType<OBJECT_T>(),
                  "First parameter must match config type of member function being created!",
                  type_id, emp::GetTypeID<OBJECT_T>() );

      // ----- Transform this function into one that ConfigTypeInfo can make use of ----
      MemberFunInfo::fun_t member_fun =
        [name,fun](ConfigType & obj, const emp::vector<entry_ptr_t> & args) -> RETURN_T {
          // Make sure we can convert the obj into the correct type.
          emp::Ptr<OBJECT_T> typed_ptr = dynamic_cast<OBJECT_T*>(&obj);

          emp_assert(typed_ptr, "Internal Error: member function called on wrong object type!",
                     name);

          // Make sure we have the correct number of arguments.
          if (args.size() != sizeof...(PARAM_Ts)) {
            std::cerr << "Error in call to function '" << name
              << "'; expected " << sizeof...(PARAM_Ts)
              << " arguments, but received " << args.size() << "."
              << std::endl;
          }
          //@CAO should collect file position information for the above errors.

          // Call the provided function and return the result.
          int arg_id = 0;
          return fun( *typed_ptr, args[arg_id++]->As<PARAM_Ts>()... );
        };

      // Add this member function to the library we are building.
      member_funs.emplace_back(name, desc, member_fun);
    }
  };

}
#endif
