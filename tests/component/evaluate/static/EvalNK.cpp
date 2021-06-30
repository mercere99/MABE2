
/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file EvalNK.cpp 
 *  @brief Tests the data collection tools with various data types and edge cases
 */

// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// Empirical tools
#include "emp/base/vector.hpp"
// MABE
#include "../../../../source/evaluate/static/EvalNK.hpp"


TEST_CASE("EvalNK_test-case", "[evaluate/static]"){
  {
    std::function<std::string(emp::vector<int>)> fun = 
        emp::BuildCollectFun_Max<int, emp::vector<int>>([](int i){ return i;});
    emp::vector v{4,8,8,2,3};
    REQUIRE(fun(v) == "8");
    emp::vector v2{0,1,2,3};
    REQUIRE(fun(v2) == "3");
    emp::vector v3{-1,-2,-3};
    REQUIRE(fun(v3) == "-1");
  }
}
