/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  MABE.cpp
 *  @brief Tests for the main MABE object 
 */

// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// Empirical tools
#include "emp/bits/BitVector.hpp"
#include "emp/config/ArgManager.hpp"
#include "emp/math/Random.hpp"
// MABE
#include "../../../source/core/MABE.hpp"
#include "../../../source/core/EmptyOrganism.hpp"
#include "../../../source/modules.hpp"


TEST_CASE("MABE", "[core]"){
  {
    mabe::MABE control(0, NULL);
    REQUIRE(control.GetUpdate() == 0);
    REQUIRE(1 + 1 == 3); // Should fail
  }
}
