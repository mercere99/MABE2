
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

int evalPacking(size_t brick_s, size_t packing_s, const emp::BitVector bits) {
    size_t brick_size = brick_s;
    size_t packing_size = packing_s;

    if (bits.GetSize() < brick_size) {
        return 0;
    }

    int packed = 0; // number of correctly packed bricks

    size_t ones_count = 0;
    size_t zeros_count = 0;
    int check_step = 1; // 0 = count front packing, 1 = count brick, 2 = count back packing, 3 = all elements found
    
    for (size_t i = 0; i < bits.size(); i++) {
        if (check_step == 0 || check_step == 2) {
            if (bits[i] == 0) {
                zeros_count++;
            }
            if (zeros_count == packing_size) {
                zeros_count = 0;
                check_step++;
            }
            // one found, restart search for front packing
            else if (bits[i] == 1) {
                zeros_count = 0;
                check_step = 0;
            }
        }
        // looking for brick
        else if (check_step == 1) {
            if (bits[i] == 1) {
                ones_count++;
                // full brick found, begin looking for zeros
                if (ones_count == brick_size) {
                    ones_count = 0;
                    check_step++;
                }
            }
            // zero found, begin looking for front packing
            else if (bits[i] == 0) {
                ones_count = 0;
                zeros_count = 1;
                check_step = 0;
            }
        }
        if (check_step == 3) {
            packed++;
            check_step = 1;
        }
    }

    return packed;
}

TEST_CASE("EvalPacking_test-case", "[evaluate/static]"){
  {
   const emp::BitVector & bits = emp::BitVector("00");
   REQUIRE(evalPacking(3, 2, bits) == 0);
   const emp::BitVector & bits1 = emp::BitVector("000011110000");
   REQUIRE(evalPacking(4, 4, bits1) == 1);
   const emp::BitVector & bits2 = emp::BitVector("111100111100");
   REQUIRE(evalPacking(4, 2, bits2) == 2);
   const emp::BitVector & bits3 = emp::BitVector("00000111011100");
   REQUIRE_FALSE(evalPacking(3, 2, bits3) == 2);
   const emp::BitVector & bits4 = emp::BitVector("111111111111111");
   REQUIRE_FALSE(evalPacking(3, 0, bits4) == 5);
  }
}