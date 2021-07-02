
/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file EvalPacking.cpp 
 *  @brief Tests the Nk evaluation method tool with various edge cases
 */

// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// Empirical tools
#include "emp/base/vector.hpp"
// MABE
#include "../../../../source/evaluate/static/EvalNK.hpp"
#include "evaluate/static/EvalPacking.hpp"

TEST_CASE("EvalPacking_test-case", "[evaluate/static]"){
  {
  mabe::MABE control = mabe::MABE(0, NULL);
  control.AddPopulation("pop");
  mabe::EvalPacking packing(control);
  // packing.evaluate(brick_size, packing_size, bits)

   const emp::BitVector & bits = emp::BitVector("00");
   const emp::BitVector & bits1 = emp::BitVector("000011110000");
   const emp::BitVector & bits2 = emp::BitVector("11110000111100");
   const emp::BitVector & bits3 = emp::BitVector("00000111011100");
   const emp::BitVector & bits4 = emp::BitVector("111111111111111");


   // bits length too small 
   REQUIRE(packing.evaluate(3, 2, bits) == 0);
   REQUIRE(packing.evaluate(20, 0, bits4) == 0);

   // standard packed bricks 
   REQUIRE(packing.evaluate(4, 4, bits1) == 1);
   REQUIRE(packing.evaluate(4, 2, bits2) == 2);

    std::cout << packing.evaluate(3, 0, bits4) << std::endl;
   REQUIRE_FALSE(packing.evaluate(3, 2, bits3) == 2);

   // no packing
   //REQUIRE(packing.evaluate(3, 0, bits4) == 5);
   // no bricks
   REQUIRE(packing.evaluate(0, 3, bits4) == 0);
  }
}