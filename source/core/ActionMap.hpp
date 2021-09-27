
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
#include "emp/base/assert.hpp"
#include "emp/base/vector.hpp"
#include "emp/base/unordered_map.hpp"
#include "emp/hardware/AvidaGP.hpp"
#include "emp/functional/AnyFunction.hpp"
#include "emp/meta/TypePack.hpp"

namespace mabe {

  struct Action{
    std::string name;
    emp::AnyFunction function;
    Action(const std::string& s, emp::AnyFunction f) : name(s), function(f) { ; }
  };
  class ActionMap : public emp::unordered_map<emp::TypeID, emp::vector<mabe::Action>>{
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

}
#endif
