/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file EvalPathFollow.cpp 
 *  @brief Test file for path follow evaluator.
 */

#define TDEBUG 1
// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// Empirical tools
#include "emp/base/vector.hpp"
#include "emp/base/assert.hpp"
// MABE
#include "evaluate/games/EvalPathFollow.hpp"

TEST_CASE("EvalPathFollow_PathFollowEvaluator", "[evaluate/games]"){
  { // LoadMap
    emp::Random rand(100);
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator(rand);
    evaluator.LoadMap("path_follow_files/test_map_straight.txt");
    CHECK(evaluator.GetNumMaps() == 1); 
    CHECK(evaluator.path_data_vec.size() == 1);
    CHECK(evaluator.path_data_vec[0].grid.GetWidth() == 5);  // Num cols
    CHECK(evaluator.path_data_vec[0].grid.GetHeight() == 5); // Num rows
    { // Check each individual tile, but hide in a fold for readability in most editors
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(0,0) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(1,0) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(2,0) == 'v');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(3,0) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(4,0) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(0,1) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(1,1) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(2,1) == '+');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(3,1) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(4,1) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(0,2) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(1,2) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(2,2) == '+');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(3,2) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(4,2) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(0,3) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(1,3) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(2,3) == '+');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(3,3) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(4,3) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(0,4) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(1,4) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(2,4) == 'X');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(3,4) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(4,4) == '.');
    }
    CHECK(evaluator.path_data_vec[0].start_x == 2);
    CHECK(evaluator.path_data_vec[0].start_y == 0);
    CHECK(evaluator.path_data_vec[0].start_facing == 5);
    CHECK(evaluator.path_data_vec[0].path_length == 4);
    // Load new map with turns
    evaluator.LoadMap("path_follow_files/test_map_turns.txt");
    CHECK(evaluator.GetNumMaps() == 2); 
    CHECK(evaluator.path_data_vec.size() == 2);
    CHECK(evaluator.path_data_vec[1].grid.GetHeight() == 11); // Num rows 
    CHECK(evaluator.path_data_vec[1].grid.GetWidth() == 11);  // Num cols
    { // Check each individual tile, but hide in a fold for readability in most editors
      // Row 0
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,0) == '.');
      // Row 1
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,1) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,1) == '>');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,1) == 'R');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,1) == '.');
      // Row 2
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,2) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,2) == '.');
      // Row 3
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,3) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,3) == 'R');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,3) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,3) == 'R');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,3) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,3) == 'L');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,3) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,3) == 'L');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,3) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,3) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,3) == '.');
      // Row 4
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,4) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,4) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,4) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,4) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,4) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,4) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,4) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,4) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,4) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,4) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,4) == '.');
      // Row 5
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,5) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,5) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,5) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,5) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,5) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,5) == 'X');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,5) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,5) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,5) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,5) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,5) == '.');
      // Row 6
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,6) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,6) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,6) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,6) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,6) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,6) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,6) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,6) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,6) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,6) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,6) == '.');
      // Row 7
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,7) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,7) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,7) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,7) == 'L');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,7) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,7) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,7) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,7) == 'L');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,7) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,7) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,7) == '.');
      // Row 8
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,8) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,8) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,8) == '.');
      // Row 9
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,9) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,9) == 'R');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,9) == 'R');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,9) == '.');
      // Row 10
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10, 10) == '.');
    }
    CHECK(evaluator.path_data_vec[1].start_x == 1);
    CHECK(evaluator.path_data_vec[1].start_y == 1);
    CHECK(evaluator.path_data_vec[1].start_facing == 3);
    CHECK(evaluator.path_data_vec[1].path_length == 48);
  }
  { // LoadAllMaps
    emp::Random rand(200);
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator(rand);
    evaluator.LoadAllMaps("path_follow_files/test_map_straight.txt;path_follow_files/test_map_turns.txt");
    CHECK(evaluator.GetNumMaps() == 2); 
    CHECK(evaluator.path_data_vec.size() == 2);
    CHECK(evaluator.path_data_vec[0].grid.GetWidth() == 5);  // Num cols
    CHECK(evaluator.path_data_vec[0].grid.GetHeight() == 5); // Num rows
    { // Check each individual tile, but hide in a fold for readability in most editors
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(0,0) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(1,0) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(2,0) == 'v');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(3,0) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(4,0) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(0,1) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(1,1) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(2,1) == '+');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(3,1) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(4,1) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(0,2) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(1,2) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(2,2) == '+');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(3,2) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(4,2) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(0,3) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(1,3) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(2,3) == '+');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(3,3) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(4,3) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(0,4) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(1,4) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(2,4) == 'X');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(3,4) == '.');
      CHECK(evaluator.path_data_vec[0].grid.GetSymbol(4,4) == '.');
    }
    CHECK(evaluator.path_data_vec[0].start_x == 2);
    CHECK(evaluator.path_data_vec[0].start_y == 0);
    CHECK(evaluator.path_data_vec[0].start_facing == 5);
    CHECK(evaluator.path_data_vec[0].path_length == 4);
    // Load new map with turns
    evaluator.LoadMap("path_follow_files/test_map_turns.txt");
    CHECK(evaluator.path_data_vec[1].grid.GetHeight() == 11); // Num rows 
    CHECK(evaluator.path_data_vec[1].grid.GetWidth() == 11);  // Num cols
    { // Check each individual tile, but hide in a fold for readability in most editors
      // Row 0
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,0) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,0) == '.');
      // Row 1
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,1) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,1) == '>');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,1) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,1) == 'R');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,1) == '.');
      // Row 2
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,2) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,2) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,2) == '.');
      // Row 3
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,3) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,3) == 'R');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,3) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,3) == 'R');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,3) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,3) == 'L');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,3) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,3) == 'L');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,3) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,3) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,3) == '.');
      // Row 4
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,4) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,4) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,4) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,4) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,4) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,4) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,4) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,4) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,4) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,4) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,4) == '.');
      // Row 5
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,5) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,5) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,5) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,5) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,5) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,5) == 'X');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,5) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,5) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,5) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,5) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,5) == '.');
      // Row 6
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,6) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,6) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,6) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,6) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,6) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,6) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,6) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,6) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,6) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,6) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,6) == '.');
      // Row 7
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,7) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,7) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,7) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,7) == 'L');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,7) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,7) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,7) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,7) == 'L');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,7) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,7) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,7) == '.');
      // Row 8
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,8) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,8) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,8) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,8) == '.');
      // Row 9
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0,9) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1,9) == 'R');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8,9) == '+');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9,9) == 'R');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10,9) == '.');
      // Row 10
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(0, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(1, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(2, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(3, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(4, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(5, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(6, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(7, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(8, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(9, 10) == '.');
      CHECK(evaluator.path_data_vec[1].grid.GetSymbol(10, 10) == '.');
    }
    CHECK(evaluator.path_data_vec[1].start_x == 1);
    CHECK(evaluator.path_data_vec[1].start_y == 1);
    CHECK(evaluator.path_data_vec[1].start_facing == 3);
    CHECK(evaluator.path_data_vec[1].path_length == 48);
  }
  { // GetCurrentPosScore
    emp::Random rand(300);
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator(rand);
    evaluator.LoadAllMaps("path_follow_files/test_map_straight.txt;path_follow_files/test_map_turns.txt");
    mabe::PathFollowState state;
    evaluator.randomize_cues = false;
    evaluator.InitializeState(state);
    state.cur_map_idx = 0;
    CHECK(evaluator.GetNumMaps() == 2); 
    CHECK(evaluator.path_data_vec[0].grid.GetWidth() == 5);  // Num cols
    CHECK(evaluator.path_data_vec[0].grid.GetHeight() == 5); // Num rows
    { // Check each individual tile, but hide in a fold for readability in most editors
      state.status.Set(0,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(1,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(2,0,1); CHECK(evaluator.GetCurrentPosScore(state) ==  1);
      state.status.Set(3,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(4,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(0,1,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(1,1,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(2,1,1); CHECK(evaluator.GetCurrentPosScore(state) ==  1);
      state.status.Set(3,1,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(4,1,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(0,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(1,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(2,2,1); CHECK(evaluator.GetCurrentPosScore(state) ==  1);
      state.status.Set(3,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(4,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(0,3,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(1,3,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(2,3,1); CHECK(evaluator.GetCurrentPosScore(state) ==  1);
      state.status.Set(3,3,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(4,3,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(0,4,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(1,4,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(2,4,1); CHECK(evaluator.GetCurrentPosScore(state) ==  1);
      state.status.Set(3,4,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(4,4,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
    }
    state.cur_map_idx = 1;
    evaluator.InitializeState(state, false);
    CHECK(evaluator.path_data_vec[1].grid.GetWidth() == 11);  // Num cols
    CHECK(evaluator.path_data_vec[1].grid.GetHeight() == 11); // Num rows
    { // Check each individual tile, but hide in a fold for readability in most editors
      // Row 0
      state.status.Set( 0,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 1,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 2,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 3,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 4,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 5,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 6,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 7,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 8,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 9,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(10,0,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 1
      state.status.Set( 0,1,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 1,1,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 2,1,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 3,1,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 4,1,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 5,1,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 6,1,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 7,1,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 8,1,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 9,1,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set(10,1,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 2
      state.status.Set( 0,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 1,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 2,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 3,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 4,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 5,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 6,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 7,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 8,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 9,2,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set(10,2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 3
      state.status.Set( 0,3,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 1,3,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 2,3,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 3,3,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 4,3,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 5,3,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 6,3,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 7,3,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 8,3,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 9,3,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set(10,3,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 4
      state.status.Set( 0,4,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 1,4,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 2,4,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 3,4,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 4,4,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 5,4,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 6,4,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 7,4,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 8,4,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 9,4,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set(10,4,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 5
      state.status.Set( 0,5,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 1,5,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 2,5,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 3,5,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 4,5,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 5,5,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 6,5,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 7,5,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 8,5,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 9,5,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set(10,5,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 6
      state.status.Set( 0,6,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 1,6,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 2,6,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 3,6,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 4,6,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 5,6,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 6,6,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 7,6,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 8,6,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 9,6,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set(10,6,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 7
      state.status.Set( 0,7,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 1,7,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 2,7,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 3,7,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 4,7,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 5,7,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 6,7,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 7,7,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 8,7,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 9,7,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set(10,7,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 8
      state.status.Set( 0,8,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 1,8,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 2,8,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 3,8,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 4,8,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 5,8,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 6,8,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 7,8,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 8,8,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 9,8,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set(8,10,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 9
      state.status.Set( 0,9,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 1,9,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 2,9,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 3,9,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 4,9,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 5,9,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 6,9,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 7,9,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 8,9,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set( 9,9,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.status.Set(10,9,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 10
      state.status.Set( 0,10,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 1,10,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 2,10,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 3,10,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 4,10,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 5,10,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 6,10,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 7,10,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 8,10,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set( 9,10,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.status.Set(10,10,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
    }
  }
  { // Initialize to first map without randomizing cues
    emp::assert_clear();
    emp::Random rand(400);
    mabe::PathFollowEvaluator evaluator(rand); 
    evaluator.randomize_cues = false;
    evaluator.LoadMap("path_follow_files/test_map_turns.txt");
    mabe::PathFollowState state;
    evaluator.InitializeState(state);
    CHECK(emp::assert_last_fail == 0); // Ensure no assertions were triggered
    emp::assert_clear();
    CHECK(state.cur_map_idx == 0);
    CHECK(state.visited_tiles.GetSize() == 121); // 11x11 
    CHECK(state.status.GetX() == evaluator.path_data_vec[0].start_x);
    CHECK(state.status.GetY() == evaluator.path_data_vec[0].start_y);
    CHECK(state.status.GetFacing() == static_cast<size_t>(evaluator.path_data_vec[0].start_facing));
    CHECK(state.raw_score == 0);
    CHECK(evaluator.GetNormalizedScore(state) == 0);
    CHECK(state.empty_cue == 1);
    CHECK(state.forward_cue == 2);
    CHECK(state.left_cue == 3);
    CHECK(state.right_cue == 4);
  }
  { // Initialize to first map and randomize cues
    emp::assert_clear();
    emp::Random rand(500);
    mabe::PathFollowEvaluator evaluator(rand); 
    CHECK(evaluator.randomize_cues); // Should default to true
    CHECK(emp::assert_last_fail == 0); // Ensure no assertions were triggered
    emp::assert_clear();
    CHECK(evaluator.randomize_cues); // Ensure it didn't change with InitializeRandom
    evaluator.LoadMap("path_follow_files/test_map_turns.txt");
    mabe::PathFollowState state;
    evaluator.InitializeState(state);
    CHECK(state.cur_map_idx == 0);
    CHECK(state.visited_tiles.GetSize() == 121); // 11x11 
    CHECK(state.status.GetX() == evaluator.path_data_vec[0].start_x);
    CHECK(state.status.GetY() == evaluator.path_data_vec[0].start_y);
    CHECK(state.status.GetFacing() == static_cast<size_t>(evaluator.path_data_vec[0].start_facing));
    CHECK(state.raw_score == 0);
    CHECK(evaluator.GetNormalizedScore(state) == 0);
    CHECK(state.forward_cue == 2805919190);
    CHECK(state.right_cue == 3616089412);
    CHECK(state.left_cue == 2739352475);
    CHECK(state.empty_cue == 2994306028);
    { // Do pairwise checks to ensure no two cues share the same value
      CHECK(state.forward_cue != state.right_cue);
      CHECK(state.forward_cue != state.left_cue);
      CHECK(state.forward_cue != state.empty_cue);
      CHECK(state.right_cue != state.left_cue);
      CHECK(state.right_cue != state.empty_cue);
      CHECK(state.left_cue != state.empty_cue);
    }
  }
  { // Initialize state to the second map 
    emp::assert_clear();
    emp::Random rand(500);
    mabe::PathFollowEvaluator evaluator(rand); 
    evaluator.LoadAllMaps(
        "path_follow_files/test_map_turns.txt;path_follow_files/test_map_straight.txt");
    evaluator.randomize_cues = false;
    mabe::PathFollowState state;
    state.cur_map_idx = 1;
    evaluator.InitializeState(state, false);
    CHECK(emp::assert_last_fail == 0); // Ensure no assertions were triggered
    emp::assert_clear();
    CHECK(state.cur_map_idx == 1);
    CHECK(state.visited_tiles.GetSize() == 25); // 5x5 
    CHECK(state.status.GetX() == evaluator.path_data_vec[1].start_x);
    CHECK(state.status.GetY() == evaluator.path_data_vec[1].start_y);
    CHECK(state.status.GetFacing() == static_cast<size_t>(evaluator.path_data_vec[1].start_facing));
    CHECK(state.raw_score == 0);
    CHECK(evaluator.GetNormalizedScore(state) == 0);
    CHECK(state.empty_cue == 1);
    CHECK(state.forward_cue == 2);
    CHECK(state.left_cue == 3);
    CHECK(state.right_cue == 4);
  }
  { // MarkVisited
    emp::assert_clear();
    emp::Random rand(600);
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator(rand);
    evaluator.LoadMap("path_follow_files/test_map_straight.txt");
    mabe::PathFollowState state;
    evaluator.InitializeState(state);
    // First tile 
    CHECK(!state.visited_tiles[0]);
    state.status.Set(0,0,1); 
    evaluator.MarkVisited(state); 
    CHECK(state.visited_tiles[0]);
    // Start tile
    CHECK(!state.visited_tiles[2]);
    state.status.Set(2,0,1); 
    evaluator.MarkVisited(state); 
    CHECK(state.visited_tiles[2]);
    // Forward tile
    CHECK(!state.visited_tiles[12]); // 2 rows down * 5 tiles/row + 2 rows over = 12
    state.status.Set(2,2,1); 
    evaluator.MarkVisited(state); 
    CHECK(state.visited_tiles[12]);
    // Finish tile
    CHECK(!state.visited_tiles[22]); // 4 rows down * 5 tiles/row + 2 rows over = 22
    state.status.Set(2,4,1); 
    evaluator.MarkVisited(state); 
    CHECK(state.visited_tiles[22]);
    // Does marking a good tile visited reduce its reward? (test on a forward tile)
    state.status.Set(2,3,1); 
    CHECK(!state.visited_tiles[17]); // 3 rows down * 5 tiles/row + 2 rows over = 17 
    CHECK(evaluator.GetCurrentPosScore(state) == 1);
    CHECK(!state.visited_tiles[17]); // Check GetCurrentPosScore does not set visited
    evaluator.MarkVisited(state); 
    CHECK(state.visited_tiles[17]);
    CHECK(evaluator.GetCurrentPosScore(state) == 0);
    // Marking an empty tile should NOT reduce its punishment.
    state.status.Set(4,4,1); 
    CHECK(!state.visited_tiles[24]); // 4 rows down * 5 tiles/row + 4 rows over = 24
    CHECK(evaluator.GetCurrentPosScore(state) == -1);
    evaluator.MarkVisited(state); 
    CHECK(state.visited_tiles[24]);
    CHECK(evaluator.GetCurrentPosScore(state) == -1);
  }
  { // Move
    emp::assert_clear();
    emp::Random rand(700);
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator(rand);
    evaluator.LoadMap("path_follow_files/test_map_straight.txt");
    mabe::PathFollowState state;
    evaluator.InitializeState(state);
    { // First move: down
      CHECK(state.status.GetX() == 2);
      CHECK(state.status.GetY() == 0);
      CHECK(state.status.GetFacing() == 5);
      CHECK(!state.visited_tiles[7]); // Destination should start as unvisited (1*5 + 2 = 7)
      evaluator.Move(state);
      CHECK(state.status.GetX() == 2);
      CHECK(state.status.GetY() == 1);
      CHECK(state.visited_tiles[7]);  // Marked new tile as visited?
      CHECK(state.raw_score == 1);       // Added reward
      CHECK(evaluator.path_data_vec[0].path_length == 4);   
      CHECK(evaluator.GetNormalizedScore(state) == 1 / 4.0); // Added reward
    }
    { // second move: right
      CHECK(state.status.GetX() == 2); // State consistent from last move
      CHECK(state.status.GetY() == 1);
      state.status.SetFacing(3); // Right
      CHECK(state.status.GetFacing() == 3);
      CHECK(!state.visited_tiles[8]); // Destination should start as unvisited (1*5 + 3 = 8)
      evaluator.Move(state);
      CHECK(state.status.GetX() == 3);
      CHECK(state.status.GetY() == 1);
      CHECK(state.visited_tiles[8]);  // Marked new tile as visited?
      CHECK(state.raw_score == 0); // Score decremented 
      CHECK(evaluator.GetNormalizedScore(state) == 0);
    }
    { // third move: left 
      CHECK(state.status.GetX() == 3); // State consistent from last move
      CHECK(state.status.GetY() == 1);
      state.status.SetFacing(7); // Left 
      CHECK(state.status.GetFacing() == 7);
      CHECK(state.visited_tiles[7]); // Destination should already be visited (1*5 + 2 = 7)
      evaluator.Move(state);
      CHECK(state.status.GetX() == 2);
      CHECK(state.status.GetY() == 1);
      CHECK(state.visited_tiles[7]);  // Should still be visited
      CHECK(state.raw_score == 0); // Score remains 0 because tile was previously visited
      CHECK(evaluator.GetNormalizedScore(state) == 0);
    }
    { // fourth move: up
      CHECK(state.status.GetX() == 2); // State consistent from last move
      CHECK(state.status.GetY() == 1);
      state.status.SetFacing(1); // Up 
      CHECK(state.status.GetFacing() == 1);
      CHECK(!state.visited_tiles[2]); // Destination shouldn't have been visited (0*5 + 2 = 2)
      evaluator.Move(state);
      CHECK(state.status.GetX() == 2);
      CHECK(state.status.GetY() == 0);
      CHECK(state.visited_tiles[2]);  // Should still be visited
      CHECK(state.raw_score == 1); // Score remains because tile was previously visited
      CHECK(evaluator.GetNormalizedScore(state) == 1 / 4.0);
    }
    { // Move backward
      CHECK(state.status.GetX() == 2); // State consistent from last move
      CHECK(state.status.GetY() == 0);
      state.status.SetFacing(1); // Up 
      CHECK(state.status.GetFacing() == 1);
      evaluator.Move(state, -1);
      CHECK(state.status.GetX() == 2); // Move backwards -> Moved down 
      CHECK(state.status.GetY() == 1);

      state.status.SetFacing(3); // Right 
      CHECK(state.status.GetFacing() == 3);
      evaluator.Move(state, -1);
      CHECK(state.status.GetX() == 1); // Move backwards -> Moved left 
      CHECK(state.status.GetY() == 1);

      state.status.SetFacing(5); // Down 
      CHECK(state.status.GetFacing() == 5);
      CHECK(!state.visited_tiles[1]); // This destination has NOT been visited (0*5 + 1 = 1)
      evaluator.Move(state, -1);
      CHECK(state.visited_tiles[1]); 
      CHECK(state.status.GetX() == 1); // Move backwards -> Moved up
      CHECK(state.status.GetY() == 0);
      CHECK(state.visited_tiles[1]); // Has now been visited 

      state.status.SetFacing(7); // Left 
      CHECK(state.status.GetFacing() == 7);
      evaluator.Move(state, -1);
      CHECK(state.status.GetX() == 2); // Move backwards -> Moved right 
      CHECK(state.status.GetY() == 0);
    }
  }
  { // RotateRight
    emp::assert_clear();
    emp::Random rand(800);
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator(rand);
    evaluator.LoadMap("path_follow_files/test_map_straight.txt");
    mabe::PathFollowState state;
    evaluator.InitializeState(state);
    CHECK(state.status.GetX() == 2);
    CHECK(state.status.GetY() == 0);
    CHECK(state.status.GetFacing() == 5); // Down
    evaluator.RotateRight(state);
    CHECK(state.status.GetX() == 2);
    CHECK(state.status.GetY() == 0);
    CHECK(state.status.GetFacing() == 7); // Left
    evaluator.RotateRight(state);
    CHECK(state.status.GetFacing() == 1); // Up 
    evaluator.RotateRight(state);
    CHECK(state.status.GetFacing() == 3); // Right
    evaluator.RotateRight(state);
    CHECK(state.status.GetFacing() == 5); // Down
  }
  { // RotateLeft
    emp::assert_clear();
    emp::Random rand(900);
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator(rand);
    evaluator.LoadMap("path_follow_files/test_map_straight.txt");
    mabe::PathFollowState state;
    evaluator.InitializeState(state);
    CHECK(state.status.GetX() == 2);
    CHECK(state.status.GetY() == 0);
    CHECK(state.status.GetFacing() == 5); // Down
    evaluator.RotateLeft(state);
    CHECK(state.status.GetX() == 2);
    CHECK(state.status.GetY() == 0);
    CHECK(state.status.GetFacing() == 3); // Left
    evaluator.RotateLeft(state);
    CHECK(state.status.GetFacing() == 1); // Up 
    evaluator.RotateLeft(state);
    CHECK(state.status.GetFacing() == 7); // Right
    evaluator.RotateLeft(state);
    CHECK(state.status.GetFacing() == 5); // Down
  }
  { // Sense
    { // Deterministic cues
      emp::assert_clear();
      emp::Random rand(1000);
      mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator(rand);
      evaluator.LoadMap("path_follow_files/test_map_turns.txt");
      evaluator.randomize_cues = false;
      mabe::PathFollowState state;
      evaluator.InitializeState(state);
      CHECK(state.empty_cue == 1);
      CHECK(state.forward_cue == 2);
      CHECK(state.left_cue == 3);
      CHECK(state.right_cue == 4);
      CHECK(state.status.GetX() == 1);
      CHECK(state.status.GetY() == 1);
      CHECK(evaluator.Sense(state) == state.forward_cue); // Start tile gives forward cue
      state.status.SetPos(2,1);
      CHECK(state.status.GetX() == 2);
      CHECK(state.status.GetY() == 1);
      CHECK(evaluator.Sense(state) == state.forward_cue); // Forward tile 
      state.status.SetPos(9,1);
      CHECK(state.status.GetX() == 9);
      CHECK(state.status.GetY() == 1);
      CHECK(evaluator.Sense(state) == state.right_cue); // Right tile 
      state.status.SetPos(7,3);
      CHECK(state.status.GetX() == 7);
      CHECK(state.status.GetY() == 3);
      CHECK(evaluator.Sense(state) == state.left_cue); // Left tile 
      state.status.SetPos(5,5);
      CHECK(state.status.GetX() == 5);
      CHECK(state.status.GetY() == 5);
      CHECK(evaluator.Sense(state) == state.forward_cue); // Finish tile gives forward cue
      state.status.SetPos(0,0);
      CHECK(state.status.GetX() == 0);
      CHECK(state.status.GetY() == 0);
      CHECK(evaluator.Sense(state) == state.empty_cue); // Empty tile
    }
    { // Random cues
      emp::assert_clear();
      emp::Random rand(1000);
      mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator(rand);
      evaluator.LoadMap("path_follow_files/test_map_turns.txt");
      evaluator.randomize_cues = true;
      mabe::PathFollowState state;
      evaluator.InitializeState(state);
      CHECK(state.empty_cue == 1090903881);
      CHECK(state.forward_cue == 4052048319);
      CHECK(state.left_cue == 3477414786);
      CHECK(state.right_cue == 1134226549);
      CHECK(state.status.GetX() == 1);
      CHECK(state.status.GetY() == 1);
      CHECK(evaluator.Sense(state) == state.forward_cue); // Start tile gives forward cue
      state.status.SetPos(2,1);
      CHECK(state.status.GetX() == 2);
      CHECK(state.status.GetY() == 1);
      CHECK(evaluator.Sense(state) == state.forward_cue); // Forward tile 
      state.status.SetPos(9,1);
      CHECK(state.status.GetX() == 9);
      CHECK(state.status.GetY() == 1);
      CHECK(evaluator.Sense(state) == state.right_cue); // Right tile 
      state.status.SetPos(7,3);
      CHECK(state.status.GetX() == 7);
      CHECK(state.status.GetY() == 3);
      CHECK(evaluator.Sense(state) == state.left_cue); // Left tile 
      state.status.SetPos(5,5);
      CHECK(state.status.GetX() == 5);
      CHECK(state.status.GetY() == 5);
      CHECK(evaluator.Sense(state) == state.forward_cue); // Finish tile gives forward cue
      state.status.SetPos(0,0);
      CHECK(state.status.GetX() == 0);
      CHECK(state.status.GetY() == 0);
      CHECK(evaluator.Sense(state) == state.empty_cue); // Empty tile
    }
  }
}
