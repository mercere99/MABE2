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
#include "emp/base/map.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/tools/string_utils.hpp"

class ConfigEntry;

namespace mabe {

  // ConfigTypeInfo tracks a particular type to be used in the configuration langauge.
  class ConfigTypeInfo {
  private:
    size_t index;
    std::string type_name;
    std::string desc;
    emp::TypeID type_id;

    using init_fun_t = std::function<ConfigType & (const std::string &)>;
    init_fun_t init_fun;

    using entry_ptr_t = emp::Ptr<ConfigEntry>;
    using member_fun_t = std::function<entry_ptr_t(const emp::vector<entry_ptr_t> &)>;
    emp::map<std::string, member_fun_t> member_funs;

    emp::vector< entry_ptr_t > member_list;           ///< Member functions for this type
    emp::map< std::string, entry_ptr_t > entry_map;   ///< Lookup table for member functions.

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
      std::function<RETURN_T(OBJECT_T &, PARAM_Ts...)> fun
    ) {
      // ----- Make sure function is legal -----
      // Is return type legal?
      static_assert(std::is_arithmetic<RETURN_T>() || std::is_same<RETURN_T, std::string>(),
                    "Config member functions must of a string or arithmetic return type");

      // Is the first parameter the correct type?
      emp_assert( type_id.IsType<OBJECT_T>(),
                  "First parameter must match config type of member function being created!",
                  type_id, emp::GetTypeID<OBJECT_T>() );

      // Are remaining parameters legal?
      constexpr bool params_ok =
        ((std::is_arithmetic<PARAM_Ts>() || std::is_same<PARAM_Ts, std::string>()) && ...);
      static_assert(params_ok, "Parameters 2+ in a member function must be string or arithmetic.");

      // ----- Transform this function into one that ConfigTypeInfo can make use of ----
      member_fun_t member_fun =
        [name,fun](ConfigType & obj, const emp::vector<entry_ptr_t> & args) {
          // Make sure we can convert the obj into the correct type.
          emp::Ptr<OBJECT_T> typed_ptr = dynamic_cast<OBJECT_T*>(&obj);

          // Make sure we have the correct number of arguments.
          if (args.size() != sizeof...(PARAM_Ts)) {
            std::cerr << "Error in call to function '" << name
              << "'; expected " << sizeof...(PARAM_Ts)
              << " arguments, but received " << args.size() << "."
              << std::endl;
          }
          //@CAO should collect file position information for the above error.

          // Call the provided function and return the result.
          int arg_id = 0;
          RETURN_T result = fun( *typed_ptr, args[arg_id++]->As<PARAM_Ts>()... );

          return result;
        };

      // Add this member function to the library we are building.
      member_funs[name] = member_fun;
    }
  };

}
#endif
