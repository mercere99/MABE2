
/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ActionMap.hpp
 *  @brief A helper class to manage the name -> function mapping of actions that organisms can take.
 *  @note Status: ALPHA
**/

#ifndef MABE_ACTION_MAP_H
#define MABE_ACTION_MAP_H


#include <functional>
#include <unordered_map>
#include "emp/base/assert.hpp"
#include "emp/base/vector.hpp"
#include "emp/hardware/AvidaGP.hpp"
#include "emp/functional/AnyFunction.hpp"
#include "emp/meta/TypePack.hpp"
#include "emp/data/DataMap.hpp"

namespace mabe {

  /*
  struct Action{
    std::string name;
    emp::AnyFunction function;
    Action(const std::string& s, emp::AnyFunction f) : name(s), function(f) { ; }
  };
  class ActionMap : public std::unordered_map<emp::TypeID, emp::vector<mabe::Action>>{
  public:
    ActionMap() { ; }

    template <typename RETURN, typename... PARAMS>
    void AddFunc(const std::string& name, const std::function<RETURN(PARAMS...)>& in_func){
      emp::TypeID func_type = emp::GetTypeID<RETURN(PARAMS...)>();
      //emp::AnyFunction any_func(in_func); 
      if(this->find(func_type) == this->end()){
        this->insert({func_type, emp::vector<mabe::Action>()});
      }
      emp::vector<Action>& action_vec = this->at(func_type);
      action_vec.emplace_back(name, emp::AnyFunction());
      action_vec[action_vec.size() - 1].function.Set(in_func);
      std::cout << "Stored: " << func_type.GetName() << std::endl;
      std::cout << "State of action vec after store:" << std::endl;
      std::cout << "[";
      for(Action& a : this->at(func_type)){
        std::cout << " " << a.name;
      }
      std::cout << "]" << std::endl;;
    }
    
    template <typename RETURN, typename... PARAMS>
    emp::vector<mabe::Action>&  GetFuncs(){
      emp::TypeID func_type = emp::GetTypeID<RETURN(PARAMS...)>();
      std::cout << "Fetching: " << func_type.GetName() << std::endl;
      return this->at(func_type);
    }
  };
  */
    
  struct Action{
    std::string name;
    emp::vector<emp::AnyFunction> function_vec;
    size_t num_args;
    emp::DataMap data;
    Action(const std::string& _name, emp::AnyFunction _func, size_t _num_args = 0) :
        name(_name),
        function_vec(),
        num_args(_num_args), 
        data(){
      function_vec.push_back(_func);
    }
    Action(const std::string& _name) :
        name(_name),
        function_vec(),
        num_args(0), 
        data(){ ; }
  };

  //TODO: Switch to std unordered map
  class ActionMap : public std::unordered_map<emp::TypeID, std::unordered_map<std::string, Action>>{
  public:
    ActionMap() { ; }



    template <typename RETURN, typename... PARAMS>
    Action& AddFunc(
        const std::string& name,
        const std::function<RETURN(PARAMS...)>& in_func,
        size_t num_args = 0){
      emp::TypeID func_type = emp::GetTypeID<RETURN(PARAMS...)>();
      //emp::AnyFunction any_func(in_func);
      if(this->find(func_type) == this->end()){
        this->insert({ func_type, std::unordered_map<std::string, Action>() });
      }
      std::unordered_map<std::string, Action>& action_map = this->at(func_type);
      if(action_map.find(name) == action_map.end()){
        action_map.insert( {name, Action(name)} );
      }
      Action& action = action_map.at(name);

      emp::vector<emp::AnyFunction>& action_vec = action.function_vec;
      action_vec.emplace_back(in_func);

      if(num_args > action.num_args) action.num_args = num_args;
      return action;
    }

    template <typename RETURN, typename... PARAMS>
    std::unordered_map<std::string, mabe::Action>&  GetFuncs(){
      emp::TypeID func_type = emp::GetTypeID<RETURN(PARAMS...)>();
      emp_assert(this->find(func_type) != this->end(), "No actions with that function signature!");
      return this->at(func_type);
    }
  };

}
#endif
