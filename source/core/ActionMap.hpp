/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  ActionMap.hpp
 *  @brief An inter-module collection of functions that can be called by organisms. Functions are accessed by their type signature. 
 *
 *  Example: In Avida, we have the IO action. 
 *  The default instruction set might create an IO action that outputs the value in the  relevant register and then inserts the next random number into the register. 
 *  However, if we want to evaluate the output, we can have another module that adds a second function to the IO action. This function *also* gets called when the IO instruction is executed, and because order is preserved, it can evaluate the output value created by the first module.
 *
 *  @note Status: ALPHA
**/

#ifndef MABE_ACTION_MAP_H
#define MABE_ACTION_MAP_H


#include <functional>
#include <unordered_map>
#include "emp/base/assert.hpp"
#include "emp/base/vector.hpp"
#include "emp/functional/AnyFunction.hpp"
#include "emp/meta/TypePack.hpp"
#include "emp/data/DataMap.hpp"

namespace mabe {
    
  /// \brief Container for the basic information of an "action" - a collection of related functions. 
  /// Actions are kept simple yet extendable. Modules can pass extra information through the datamap, but it is completely optional.  
  struct Action{
    std::string name;  ///< Human-readable name of an action
    emp::vector<emp::AnyFunction> function_vec; ///< Collection of functions associated with this action
    emp::DataMap data; ///< Generic datamap for any additional data a module wants the organism to have 

    Action(const std::string& _name, emp::AnyFunction _func) :
        name(_name),
        function_vec(),
        data(){
      function_vec.push_back(_func);
    }
    Action(const std::string& _name) :
        name(_name),
        function_vec(),
        data(){ ; }
    Action() = default;
  };

  /// \brief An inter-module collection of functions that can be called by organisms. Functions are accessed by their type signature. 
  class ActionMap : public std::unordered_map<emp::TypeID, std::unordered_map<std::string, Action>>{
  public:
    ActionMap() { ; }

    /// Add a single function to the map based on the type signature and name. Expand either map if necessary.  
    template <typename RETURN, typename... PARAMS>
    Action& AddFunc(
        const std::string& name,
        const std::function<RETURN(PARAMS...)>& in_func){
      // Get the type, and either create a new entry or find the existing entry in the map
      emp::TypeID func_type = emp::GetTypeID<RETURN(PARAMS...)>();
      if(this->find(func_type) == this->end()){
        this->insert({ func_type, std::unordered_map<std::string, Action>() });
      }
      std::unordered_map<std::string, Action>& action_map = this->at(func_type);
      // Repeat the process in the inner map using the passed name
      if(action_map.find(name) == action_map.end()){
        action_map.insert( {name, Action(name)} );
      }
      Action& action = action_map.at(name);

      emp::vector<emp::AnyFunction>& action_vec = action.function_vec;
      action_vec.emplace_back(in_func);
      return action;
    }

    /// Return the name->action map for the given type
    template <typename RETURN, typename... PARAMS>
    std::unordered_map<std::string, mabe::Action>&  GetFuncs(){
      emp::TypeID func_type = emp::GetTypeID<RETURN(PARAMS...)>();
      emp_assert(this->find(func_type) != this->end(), "No actions with that function signature!");
      return this->at(func_type);
    }
  };

}
#endif
