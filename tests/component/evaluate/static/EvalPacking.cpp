/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file EvalPacking.cpp 
 *  @brief Tests the evaluate function for EvalPacking.hpp. 
 */

// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// Empirical tools
#include "emp/base/vector.hpp"
// MABE
#include "evaluate/static/EvalPacking.hpp"


TEST_CASE("evaluate_basic tests", "[evaluate/static]"){
  {

    //remember to create addPopulation otherwise evalPacking freaks out
    mabe::MABE control = mabe::MABE(0, NULL);
    control.AddPopulation("fake pop");
    mabe::EvalPacking packing(control);


    // packing.Evaluate (bits, num_zeros, num_ones)

    const emp::BitVector & bits0 = emp::BitVector(""); 
    const emp::BitVector & bits1 = emp::BitVector("00000000"); 
    const emp::BitVector & bits2 = emp::BitVector("11111111"); 
    const emp::BitVector & bits3 = emp::BitVector("100100100"); 
    const emp::BitVector & bits4 = emp::BitVector("0011100"); 
    const emp::BitVector & bits5 = emp::BitVector("011100111011100"); 

    // //check if bitvector is empty
    REQUIRE(packing.Evaluate(bits0, 1, 2) == 0.0); //should this throw an exception?

    //check basic implementation is working
    //REQUIRE(packing.Evaluate(bits1, 2, 0) == 7.0); //?????
    //REQUIRE(packing.Evaluate(bits2, 0, 2) == 4.0); //?????
    REQUIRE(packing.Evaluate(bits3, 2, 1) == 2.0); 
    REQUIRE(packing.Evaluate(bits4, 2, 3) == 1.0); 
    
    //check number of preceding 0s make no difference
    //REQUIRE(packing.Evaluate(bits5, 1, 3) == 3.0);
    //REQUIRE(packing.Evaluate(bits4, 1, 3) == 1.0);

    //check that when no packages, fitness is 0
    REQUIRE(packing.Evaluate(bits1, 1, 2) == 0.0); 
    REQUIRE(packing.Evaluate(bits2, 2, 1) == 0.0); 
    REQUIRE(packing.Evaluate(bits3, 8, 1) == 0.0); 
    

    

  }
}
