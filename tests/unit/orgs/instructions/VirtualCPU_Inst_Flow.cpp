/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2022.
 *
 *  @file  VirtualCPUOrg_Inst_Flow.cpp
 *  @brief Test functionality of Flow instructions for VirtualCPUOrgs. 
 */

// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "orgs/VirtualCPUOrg.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Flow.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Nop.hpp"
#include "orgs/instructions/VirtualCPU_Inst_IO.hpp"

//  [ ] SetupConfig 
//  [ ] SetupModule
//  [ ] SetupFuncs
//  [ ] Generated actions

template<typename T>
T& GetConfiguredRef(
    mabe::MABE& control, 
    const std::string& type_name, 
    const std::string& var_name, 
    emplode::Symbol_Scope& scope){
  emplode::Symbol_Object& symbol_obj = 
      control.GetConfigScript().GetSymbolTable().MakeObjSymbol(type_name, var_name, scope);
  return *dynamic_cast<T*>(symbol_obj.GetObjectPtr().Raw());
}

TEST_CASE("VirtualCPUOrg_Inst_Flow_Main", "[orgs/instructions]"){
}
