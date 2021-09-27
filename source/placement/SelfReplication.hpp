/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  SelfReplication.hpp
 *  @brief ...
 */

#ifndef MABE_SELF_REPLICATION_H
#define MABE_SELF_REPLICATION_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"
#include "../orgs/GenericAvidaOrg.hpp"

namespace mabe {

  class SelfReplication : public Module {
  private:
    int pop_id = 0;
    std::function<void(GenericAvidaOrg&)> func;

  public:
    SelfReplication(mabe::MABE & control,
                    const std::string & name="SelfReplication",
                    const std::string & desc="test")
      : Module(control, name, desc)
    { }
    ~SelfReplication() { }

    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", "Population(s) to manage.");
    }

    static void TestFunc(emp::AvidaGP& org){
      static size_t counter = 0;
      ++counter;
    }

    void SetupModule() override {
      static size_t counter = 0;
      //static std::function<void(emp::AvidaGP&)> test_func = [&counter](emp::AvidaGP& org){
      func = [&counter](GenericAvidaOrg& org){
        std::cout << "Counter: " << ++counter << std::endl;
      }; 
      std::cout << "Population id:" << pop_id << std::endl;
      ActionMap& action_map = control.GetActionMap(pop_id);
      action_map.AddFunc<void,GenericAvidaOrg&>("Counter", func);
    }

  };

  MABE_REGISTER_MODULE(SelfReplication, "");
}

#endif
