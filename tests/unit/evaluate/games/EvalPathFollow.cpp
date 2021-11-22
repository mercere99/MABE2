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
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator();
    evaluator.LoadMap("path_follow_files/test_map_straight.txt");
    CHECK(evaluator.num_maps == 1); 
    CHECK(evaluator.tile_map_vec.size() == 1);
    CHECK(evaluator.tile_map_vec[0].size() == 5);    // Num rows
    CHECK(evaluator.tile_map_vec[0][0].size() == 5); // Num cols
    { // Check each individual tile, but hide in a fold for readability in most editors
      CHECK(evaluator.tile_map_vec[0][0][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][0][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][0][2] == mabe::PathFollowEvaluator::Tile::START);
      CHECK(evaluator.tile_map_vec[0][0][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][0][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][1][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][1][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][1][2] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[0][1][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][1][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][2][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][2][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][2][2] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[0][2][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][2][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][3][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][3][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][3][2] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[0][3][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][3][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][4][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][4][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][4][2] == mabe::PathFollowEvaluator::Tile::FINISH);
      CHECK(evaluator.tile_map_vec[0][4][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][4][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
    }
    CHECK(evaluator.start_pos_vec.size() == 1);
    CHECK(evaluator.start_pos_vec[0] == emp::Point2D(0, 2));
    CHECK(evaluator.start_dir_vec.size() == 1);
    CHECK(evaluator.start_dir_vec[0] == emp::Point2D(1, 0));
    CHECK(evaluator.path_length_vec.size() == 1);
    CHECK(evaluator.path_length_vec[0] == 4);
    // Load new map with turns
    evaluator.LoadMap("path_follow_files/test_map_turns.txt");
    CHECK(evaluator.num_maps == 2); 
    CHECK(evaluator.tile_map_vec.size() == 2);
    CHECK(evaluator.tile_map_vec[1].size() == 11);    // Num rows
    CHECK(evaluator.tile_map_vec[1][0].size() == 11); // Num cols
    { // Check each individual tile, but hide in a fold for readability in most editors
      // Row 0
      CHECK(evaluator.tile_map_vec[1][0][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][5] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][7] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][9] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 1
      CHECK(evaluator.tile_map_vec[1][1][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][1][1] == mabe::PathFollowEvaluator::Tile::START);
      CHECK(evaluator.tile_map_vec[1][1][2] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][3] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][4] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][5] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][6] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][7] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][8] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][9] == mabe::PathFollowEvaluator::Tile::RIGHT);
      CHECK(evaluator.tile_map_vec[1][1][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 2
      CHECK(evaluator.tile_map_vec[1][2][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][5] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][7] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][2][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 3
      CHECK(evaluator.tile_map_vec[1][3][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][3][1] == mabe::PathFollowEvaluator::Tile::RIGHT);
      CHECK(evaluator.tile_map_vec[1][3][2] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][3][3] == mabe::PathFollowEvaluator::Tile::RIGHT);
      CHECK(evaluator.tile_map_vec[1][3][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][3][5] == mabe::PathFollowEvaluator::Tile::LEFT);
      CHECK(evaluator.tile_map_vec[1][3][6] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][3][7] == mabe::PathFollowEvaluator::Tile::LEFT);
      CHECK(evaluator.tile_map_vec[1][3][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][3][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][3][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 4
      CHECK(evaluator.tile_map_vec[1][4][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][4][1] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][4][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][4][3] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][4][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][4][5] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][4][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][4][7] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][4][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][4][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][4][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 5
      CHECK(evaluator.tile_map_vec[1][5][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][5][1] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][5][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][5][3] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][5][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][5][5] == mabe::PathFollowEvaluator::Tile::FINISH);
      CHECK(evaluator.tile_map_vec[1][5][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][5][7] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][5][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][5][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][5][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 6
      CHECK(evaluator.tile_map_vec[1][6][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][6][1] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][6][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][6][3] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][6][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][6][5] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][6][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][6][7] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][6][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][6][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][6][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 7
      CHECK(evaluator.tile_map_vec[1][7][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][7][1] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][7][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][7][3] == mabe::PathFollowEvaluator::Tile::LEFT);
      CHECK(evaluator.tile_map_vec[1][7][4] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][7][5] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][7][6] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][7][7] == mabe::PathFollowEvaluator::Tile::LEFT);
      CHECK(evaluator.tile_map_vec[1][7][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][7][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][7][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 8
      CHECK(evaluator.tile_map_vec[1][8][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][1] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][8][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][5] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][7] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][8][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 9
      CHECK(evaluator.tile_map_vec[1][9][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][9][1] == mabe::PathFollowEvaluator::Tile::RIGHT);
      CHECK(evaluator.tile_map_vec[1][9][2] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][3] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][4] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][5] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][6] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][7] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][8] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][9] == mabe::PathFollowEvaluator::Tile::RIGHT);
      CHECK(evaluator.tile_map_vec[1][9][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 10
      CHECK(evaluator.tile_map_vec[1][10][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][5] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][7] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][9] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
    }
    CHECK(evaluator.start_pos_vec.size() == 2);
    CHECK(evaluator.start_pos_vec[1] == emp::Point2D(1, 1));
    CHECK(evaluator.start_dir_vec.size() == 2);
    CHECK(evaluator.start_dir_vec[1] == emp::Point2D(0, 1));
    CHECK(evaluator.path_length_vec.size() == 2);
    CHECK(evaluator.path_length_vec[1] == 48);
  }
  { // LoadAllMaps
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator();
    evaluator.LoadAllMaps("path_follow_files/test_map_straight.txt;path_follow_files/test_map_turns.txt");
    CHECK(evaluator.num_maps == 2); 
    CHECK(evaluator.tile_map_vec.size() == 2);
    CHECK(evaluator.tile_map_vec[0].size() == 5);    // Num rows
    CHECK(evaluator.tile_map_vec[0][0].size() == 5); // Num cols
    { // Check each individual tile, but hide in a fold for readability in most editors
      CHECK(evaluator.tile_map_vec[0][0][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][0][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][0][2] == mabe::PathFollowEvaluator::Tile::START);
      CHECK(evaluator.tile_map_vec[0][0][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][0][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][1][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][1][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][1][2] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[0][1][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][1][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][2][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][2][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][2][2] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[0][2][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][2][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][3][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][3][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][3][2] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[0][3][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][3][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][4][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][4][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][4][2] == mabe::PathFollowEvaluator::Tile::FINISH);
      CHECK(evaluator.tile_map_vec[0][4][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[0][4][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
    }
    CHECK(evaluator.start_pos_vec.size() == 2);
    CHECK(evaluator.start_pos_vec[0] == emp::Point2D(0, 2));
    CHECK(evaluator.start_dir_vec.size() == 2);
    CHECK(evaluator.start_dir_vec[0] == emp::Point2D(1, 0));
    CHECK(evaluator.path_length_vec.size() == 2);
    CHECK(evaluator.path_length_vec[0] == 4);
    // Check second map
    CHECK(evaluator.tile_map_vec.size() == 2);
    CHECK(evaluator.tile_map_vec[1].size() == 11);    // Num rows
    CHECK(evaluator.tile_map_vec[1][0].size() == 11); // Num cols
    { // Check each individual tile, but hide in a fold for readability in most editors
      // Row 0
      CHECK(evaluator.tile_map_vec[1][0][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][5] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][7] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][9] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][0][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 1
      CHECK(evaluator.tile_map_vec[1][1][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][1][1] == mabe::PathFollowEvaluator::Tile::START);
      CHECK(evaluator.tile_map_vec[1][1][2] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][3] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][4] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][5] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][6] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][7] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][8] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][1][9] == mabe::PathFollowEvaluator::Tile::RIGHT);
      CHECK(evaluator.tile_map_vec[1][1][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 2
      CHECK(evaluator.tile_map_vec[1][2][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][5] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][7] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][2][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][2][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 3
      CHECK(evaluator.tile_map_vec[1][3][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][3][1] == mabe::PathFollowEvaluator::Tile::RIGHT);
      CHECK(evaluator.tile_map_vec[1][3][2] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][3][3] == mabe::PathFollowEvaluator::Tile::RIGHT);
      CHECK(evaluator.tile_map_vec[1][3][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][3][5] == mabe::PathFollowEvaluator::Tile::LEFT);
      CHECK(evaluator.tile_map_vec[1][3][6] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][3][7] == mabe::PathFollowEvaluator::Tile::LEFT);
      CHECK(evaluator.tile_map_vec[1][3][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][3][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][3][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 4
      CHECK(evaluator.tile_map_vec[1][4][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][4][1] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][4][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][4][3] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][4][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][4][5] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][4][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][4][7] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][4][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][4][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][4][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 5
      CHECK(evaluator.tile_map_vec[1][5][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][5][1] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][5][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][5][3] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][5][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][5][5] == mabe::PathFollowEvaluator::Tile::FINISH);
      CHECK(evaluator.tile_map_vec[1][5][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][5][7] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][5][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][5][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][5][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 6
      CHECK(evaluator.tile_map_vec[1][6][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][6][1] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][6][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][6][3] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][6][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][6][5] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][6][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][6][7] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][6][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][6][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][6][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 7
      CHECK(evaluator.tile_map_vec[1][7][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][7][1] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][7][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][7][3] == mabe::PathFollowEvaluator::Tile::LEFT);
      CHECK(evaluator.tile_map_vec[1][7][4] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][7][5] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][7][6] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][7][7] == mabe::PathFollowEvaluator::Tile::LEFT);
      CHECK(evaluator.tile_map_vec[1][7][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][7][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][7][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 8
      CHECK(evaluator.tile_map_vec[1][8][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][1] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][8][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][5] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][7] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][8][9] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][8][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 9
      CHECK(evaluator.tile_map_vec[1][9][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][9][1] == mabe::PathFollowEvaluator::Tile::RIGHT);
      CHECK(evaluator.tile_map_vec[1][9][2] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][3] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][4] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][5] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][6] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][7] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][8] == mabe::PathFollowEvaluator::Tile::FORWARD);
      CHECK(evaluator.tile_map_vec[1][9][9] == mabe::PathFollowEvaluator::Tile::RIGHT);
      CHECK(evaluator.tile_map_vec[1][9][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
      // Row 10
      CHECK(evaluator.tile_map_vec[1][10][0] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][1] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][2] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][3] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][4] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][5] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][6] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][7] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][8] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][9] == mabe::PathFollowEvaluator::Tile::EMPTY);
      CHECK(evaluator.tile_map_vec[1][10][10] == mabe::PathFollowEvaluator::Tile::EMPTY);
    }
    CHECK(evaluator.start_pos_vec[1] == emp::Point2D(1, 1));
    CHECK(evaluator.start_dir_vec[1] == emp::Point2D(0, 1));
    CHECK(evaluator.path_length_vec[1] == 48);
  }
  { // GetCurrentPosScore
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator();
    evaluator.LoadAllMaps("path_follow_files/test_map_straight.txt;path_follow_files/test_map_turns.txt");
    mabe::PathFollowState state;
    evaluator.randomize_cues = false;
    evaluator.InitializeState(state);
    state.cur_map_idx = 0;
    CHECK(evaluator.num_maps == 2); 
    CHECK(evaluator.tile_map_vec[0].size() == 5);    // Num rows
    CHECK(evaluator.tile_map_vec[0][0].size() == 5); // Num cols
    { // Check each individual tile, but hide in a fold for readability in most editors
      state.cur_pos.Set(0, 0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(0, 1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(0, 2); CHECK(evaluator.GetCurrentPosScore(state) ==  0);
      state.cur_pos.Set(0, 3); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(0, 4); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(1, 0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(1, 1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(1, 2); CHECK(evaluator.GetCurrentPosScore(state) ==  1);
      state.cur_pos.Set(1, 3); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(1, 4); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(2, 0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(2, 1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(2, 2); CHECK(evaluator.GetCurrentPosScore(state) ==  1);
      state.cur_pos.Set(2, 3); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(2, 4); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(3, 0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(3, 1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(3, 2); CHECK(evaluator.GetCurrentPosScore(state) ==  1);
      state.cur_pos.Set(3, 3); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(3, 4); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(4, 0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(4, 1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(4, 2); CHECK(evaluator.GetCurrentPosScore(state) ==  1);
      state.cur_pos.Set(4, 3); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(4, 4); CHECK(evaluator.GetCurrentPosScore(state) == -1);
    }
    state.cur_map_idx = 1;
    evaluator.InitializeState(state, false);
    CHECK(evaluator.tile_map_vec[1].size() == 11);    // Num rows
    CHECK(evaluator.tile_map_vec[1][0].size() == 11); // Num cols
    { // Check each individual tile, but hide in a fold for readability in most editors
      // Row 0
      state.cur_pos.Set(0,0);; CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(0,1);; CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(0,2);; CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(0,3);; CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(0,4);; CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(0,5);; CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(0,6);; CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(0,7);; CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(0,8);; CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(0,9);; CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(0,10);; CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 1
      state.cur_pos.Set(1,0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(1,1); CHECK(evaluator.GetCurrentPosScore(state) == 0);
      state.cur_pos.Set(1,2); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(1,3); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(1,4); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(1,5); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(1,6); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(1,7); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(1,8); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(1,9); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(1,10); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 2
      state.cur_pos.Set(2,0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(2,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(2,2); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(2,3); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(2,4); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(2,5); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(2,6); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(2,7); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(2,8); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(2,9); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(2,10); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 3
      state.cur_pos.Set(3,0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(3,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(3,2); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(3,3); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(3,4); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(3,5); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(3,6); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(3,7); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(3,8); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(3,9); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(3,10); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 4
      state.cur_pos.Set(4,0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(4,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(4,2); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(4,3); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(4,4); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(4,5); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(4,6); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(4,7); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(4,8); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(4,9); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(4,10); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 5
      state.cur_pos.Set(5,0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(5,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(5,2); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(5,3); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(5,4); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(5,5); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(5,6); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(5,7); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(5,8); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(5,9); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(5,10); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 6
      state.cur_pos.Set(6,0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(6,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(6,2); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(6,3); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(6,4); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(6,5); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(6,6); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(6,7); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(6,8); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(6,9); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(6,10); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 7
      state.cur_pos.Set(7,0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(7,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(7,2); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(7,3); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(7,4); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(7,5); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(7,6); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(7,7); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(7,8); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(7,9); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(7,10); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 8
      state.cur_pos.Set(8,0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(8,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(8,2); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(8,3); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(8,4); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(8,5); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(8,6); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(8,7); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(8,8); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(8,9); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(8,10); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 9
      state.cur_pos.Set(9,0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(9,1); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(9,2); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(9,3); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(9,4); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(9,5); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(9,6); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(9,7); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(9,8); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(9,9); CHECK(evaluator.GetCurrentPosScore(state) == 1);
      state.cur_pos.Set(9,10); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      // Row 10
      state.cur_pos.Set(10,0); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(10,1); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(10,2); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(10,3); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(10,4); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(10,5); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(10,6); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(10,7); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(10,8); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(10,9); CHECK(evaluator.GetCurrentPosScore(state) == -1);
      state.cur_pos.Set(10,10); CHECK(evaluator.GetCurrentPosScore(state) == -1);
    }
  }
  { // InitializeRandom
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator();
    emp::Random local_random;
    evaluator.InitializeRandom(local_random, true);
    CHECK(&local_random == evaluator.rand_ptr.Raw());
    CHECK(evaluator.randomize_cues);
    evaluator.InitializeRandom(local_random, false);
    CHECK(&local_random == evaluator.rand_ptr.Raw());
    CHECK(!evaluator.randomize_cues);
  }
  { // InitializeState
    { // Initialize to first map without randomizing cues
      emp::assert_clear();
      mabe::PathFollowEvaluator evaluator; 
      evaluator.randomize_cues = false;
      evaluator.LoadMap("path_follow_files/test_map_turns.txt");
      mabe::PathFollowState state;
      evaluator.InitializeState(state);
      CHECK(emp::assert_last_fail == 0); // Ensure no assertions were triggered
      emp::assert_clear();
      CHECK(state.cur_map_idx == 0);
      CHECK(state.visited_tiles.size() == 11);    // Num rows
      CHECK(state.visited_tiles[0].size() == 11); // Num cols
      CHECK(state.cur_pos == evaluator.start_pos_vec[0]);
      CHECK(state.cur_dir == evaluator.start_dir_vec[0]);
      CHECK(state.raw_score == 0);
      CHECK(state.normalized_score == 0);
      CHECK(state.forward_cue == 1);
      CHECK(state.right_cue == 2);
      CHECK(state.left_cue == 3);
      CHECK(state.empty_cue == 4);
    }
    { // Initialize to first map and randomize cues
      emp::assert_clear();
      mabe::PathFollowEvaluator evaluator; 
      CHECK(evaluator.randomize_cues); // Should default to true
      emp::Random local_rand(12); // Set a seed
      evaluator.InitializeRandom(local_rand, true);
      CHECK(emp::assert_last_fail == 0); // Ensure no assertions were triggered
      emp::assert_clear();
      CHECK(evaluator.randomize_cues); // Ensure it didn't change with InitializeRandom
      evaluator.LoadMap("path_follow_files/test_map_turns.txt");
      mabe::PathFollowState state;
      evaluator.InitializeState(state);
      CHECK(state.cur_map_idx == 0);
      CHECK(state.visited_tiles.size() == 11);    // Num rows
      CHECK(state.visited_tiles[0].size() == 11); // Num cols
      CHECK(state.cur_pos == evaluator.start_pos_vec[0]);
      CHECK(state.cur_dir == evaluator.start_dir_vec[0]);
      CHECK(state.raw_score == 0);
      CHECK(state.normalized_score == 0);
      CHECK(state.forward_cue == 4023234300);
      CHECK(state.right_cue == 2760829176);
      CHECK(state.left_cue == 843929936);
      CHECK(state.empty_cue == 136113612);
      { // Do pairwise checks to ensure no two cues share the same value
        CHECK(state.forward_cue != state.right_cue);
        CHECK(state.forward_cue != state.left_cue);
        CHECK(state.forward_cue != state.empty_cue);
        CHECK(state.right_cue != state.left_cue);
        CHECK(state.right_cue != state.empty_cue);
        CHECK(state.left_cue != state.empty_cue);
      }
    }
    { // Initialize random but keep deterministic cues
      emp::assert_clear();
      mabe::PathFollowEvaluator evaluator; 
      CHECK(evaluator.randomize_cues); // Should default to true
      emp::Random local_rand(12); // Set a seed
      evaluator.InitializeRandom(local_rand, false);
      CHECK(emp::assert_last_fail == 0); // Ensure no assertions were triggered
      emp::assert_clear();
      CHECK(!evaluator.randomize_cues); // Ensure it did get set by InitializeRandom 
      evaluator.LoadMap("path_follow_files/test_map_turns.txt");
      mabe::PathFollowState state;
      evaluator.InitializeState(state);
      CHECK(state.cur_map_idx == 0);
      CHECK(state.visited_tiles.size() == 11);    // Num rows
      CHECK(state.visited_tiles[0].size() == 11); // Num cols
      CHECK(state.cur_pos == evaluator.start_pos_vec[0]);
      CHECK(state.cur_dir == evaluator.start_dir_vec[0]);
      CHECK(state.raw_score == 0);
      CHECK(state.normalized_score == 0);
      CHECK(state.forward_cue == 1);
      CHECK(state.right_cue == 2);
      CHECK(state.left_cue == 3);
      CHECK(state.empty_cue == 4);
    }
    { // Initialize state to the second map 
      mabe::PathFollowEvaluator evaluator; 
      evaluator.LoadAllMaps(
          "path_follow_files/test_map_turns.txt;path_follow_files/test_map_straight.txt");
      evaluator.randomize_cues = false;
      mabe::PathFollowState state;
      state.cur_map_idx = 1;
      evaluator.InitializeState(state, false);
      CHECK(emp::assert_last_fail == 0); // Ensure no assertions were triggered
      CHECK(state.cur_map_idx == 1);
      CHECK(state.visited_tiles.size() == 5);    // Num rows
      CHECK(state.visited_tiles[0].size() == 5); // Num cols
      CHECK(state.cur_pos == evaluator.start_pos_vec[1]);
      CHECK(state.cur_dir == evaluator.start_dir_vec[1]);
      CHECK(state.raw_score == 0);
      CHECK(state.normalized_score == 0);
      CHECK(state.forward_cue == 1);
      CHECK(state.right_cue == 2);
      CHECK(state.left_cue == 3);
      CHECK(state.empty_cue == 4);
    }
  }
  { // MarkVisited
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator();
    evaluator.LoadMap("path_follow_files/test_map_straight.txt");
    emp::Random local_random(12);
    evaluator.InitializeRandom(local_random, false);
    mabe::PathFollowState state;
    evaluator.InitializeState(state);
    // First tile 
    CHECK(!state.visited_tiles[0][0]);
    state.cur_pos.Set(0,0); evaluator.MarkVisited(state); CHECK(state.visited_tiles[0][0]);
    // Start tile
    CHECK(!state.visited_tiles[0][2]);
    state.cur_pos.Set(0,2); evaluator.MarkVisited(state); CHECK(state.visited_tiles[0][2]);
    // Forward tile
    CHECK(!state.visited_tiles[2][2]);
    state.cur_pos.Set(2,2); evaluator.MarkVisited(state); CHECK(state.visited_tiles[2][2]);
    // Finish tile
    CHECK(!state.visited_tiles[4][2]);
    state.cur_pos.Set(4,2); evaluator.MarkVisited(state); CHECK(state.visited_tiles[4][2]);
    // Does marking a good tile visited reduce its reward? (test on a forward tile)
    state.cur_pos.Set(3,2); 
    CHECK(!state.visited_tiles[3][2]);
    CHECK(evaluator.GetCurrentPosScore(state) == 1);
    CHECK(!state.visited_tiles[3][2]); // Check GetCurrentPosScore does not set visited
    evaluator.MarkVisited(state); 
    CHECK(state.visited_tiles[3][2]);
    CHECK(evaluator.GetCurrentPosScore(state) == 0);
    // Marking an empty tile should NOT reduce its punishment.
    state.cur_pos.Set(4,4); 
    CHECK(!state.visited_tiles[4][4]);
    CHECK(evaluator.GetCurrentPosScore(state) == -1);
    evaluator.MarkVisited(state); 
    CHECK(state.visited_tiles[4][4]);
    CHECK(evaluator.GetCurrentPosScore(state) == -1);
  }
  { // Move
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator();
    evaluator.LoadMap("path_follow_files/test_map_straight.txt");
    emp::Random local_random(12);
    evaluator.InitializeRandom(local_random, false);
    mabe::PathFollowState state;
    evaluator.InitializeState(state);
    { // First move: down
      CHECK(state.cur_pos == emp::Point2D(0,2));
      CHECK(state.cur_dir == emp::Point2D(1,0));
      CHECK(!state.visited_tiles[1][2]); // Destination should start as unvisited
      evaluator.Move(state);
      CHECK(state.cur_pos == emp::Point2D(1,2));
      CHECK(state.visited_tiles[1][2]);  // Marked new tile as visited?
      CHECK(state.raw_score == 1);       // Added reward
      CHECK(evaluator.path_length_vec[0] == 4);   
      CHECK(state.normalized_score == 1 / 4.0); // Added reward
    }
    { // second move: right
      CHECK(state.cur_pos == emp::Point2D(1,2)); // State consistent from last move
      state.cur_dir.Set(0,1);
      CHECK(state.cur_dir == emp::Point2D(0,1));
      CHECK(!state.visited_tiles[1][3]); // Destination should start as unvisited
      evaluator.Move(state);
      CHECK(state.cur_pos == emp::Point2D(1,3));
      CHECK(state.visited_tiles[1][3]);  // Marked new tile as visited?
      CHECK(state.raw_score == 0);       // Score decremented 
      CHECK(state.normalized_score == 0); // Added reward
    }
    { // third move: left 
      CHECK(state.cur_pos == emp::Point2D(1,3)); // State consistent from last move
      state.cur_dir.Set(0,-1);
      CHECK(state.cur_dir == emp::Point2D(0,-1));
      CHECK(state.visited_tiles[1][2]); // Already visited 
      evaluator.Move(state);
      CHECK(state.cur_pos == emp::Point2D(1,2));
      CHECK(state.visited_tiles[1][2]);  // Should still be visited 
      CHECK(state.raw_score == 0);       // Score remains 0 because tile was previously visited
      CHECK(state.normalized_score == 0); // Added reward
    }
    { // fourth move: up
      CHECK(state.cur_pos == emp::Point2D(1,2)); // State consistent from last move
      state.cur_dir.Set(-1,0);
      CHECK(state.cur_dir == emp::Point2D(-1,0));
      CHECK(!state.visited_tiles[0][2]); // NOT visited because initial tile is not marked visited
      evaluator.Move(state);
      CHECK(state.cur_pos == emp::Point2D(0,2));
      CHECK(state.visited_tiles[0][2]);  // Should still be visited 
      CHECK(state.raw_score == 0);       // Score remains 0 because start tile convey no reward 
      CHECK(state.normalized_score == 0); // Added reward
    }
    { // Move backward
      CHECK(state.cur_pos == emp::Point2D(0,2)); // State consistent from last move
      state.cur_dir.Set(-1,0);
      CHECK(state.cur_dir == emp::Point2D(-1,0));
      evaluator.Move(state, -1);
      CHECK(state.cur_pos == emp::Point2D(1,2));
      state.cur_dir.Set(0,1);
      CHECK(state.cur_dir == emp::Point2D(0,1));
      evaluator.Move(state, -1);
      CHECK(state.cur_pos == emp::Point2D(1,1));
    }

  }
  { // RotateRight
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator();
    evaluator.LoadMap("path_follow_files/test_map_straight.txt");
    emp::Random local_random(12);
    evaluator.InitializeRandom(local_random, false);
    mabe::PathFollowState state;
    evaluator.InitializeState(state);
    CHECK(state.cur_pos == emp::Point2D(0,2));
    CHECK(state.cur_dir == emp::Point2D(1,0));  // v -- (1,  0)
    evaluator.RotateRight(state);
    CHECK(state.cur_dir == emp::Point2D(0,-1)); // < -- (0, -1)
    evaluator.RotateRight(state);
    CHECK(state.cur_dir == emp::Point2D(-1,0)); // ^ -- (-1. 0)
    evaluator.RotateRight(state);
    CHECK(state.cur_dir == emp::Point2D(0,1));  // > -- (0,  1)
    evaluator.RotateRight(state);
    CHECK(state.cur_dir == emp::Point2D(1,0));  // v -- (1,  0)
  }
  { // RotateLeft
    mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator();
    evaluator.LoadMap("path_follow_files/test_map_straight.txt");
    emp::Random local_random(12);
    evaluator.InitializeRandom(local_random, false);
    mabe::PathFollowState state;
    evaluator.InitializeState(state);
    CHECK(state.cur_pos == emp::Point2D(0,2));
    CHECK(state.cur_dir == emp::Point2D(1,0));  // v -- (1,  0)
    evaluator.RotateLeft(state);
    CHECK(state.cur_dir == emp::Point2D(0,1));  // > -- (0,  1)
    evaluator.RotateLeft(state);
    CHECK(state.cur_dir == emp::Point2D(-1,0)); // ^ -- (-1. 0)
    evaluator.RotateLeft(state);
    CHECK(state.cur_dir == emp::Point2D(0,-1)); // < -- (0, -1)
    evaluator.RotateLeft(state);
    CHECK(state.cur_dir == emp::Point2D(1,0));  // v -- (1,  0)
  }
  { // Sense
    { // Deterministic cues
      mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator();
      evaluator.LoadMap("path_follow_files/test_map_turns.txt");
      emp::Random local_random(12);
      evaluator.InitializeRandom(local_random, false);
      mabe::PathFollowState state;
      evaluator.InitializeState(state);
      CHECK(state.forward_cue == 1);
      CHECK(state.right_cue == 2);
      CHECK(state.left_cue == 3);
      CHECK(state.empty_cue == 4);
      CHECK(state.cur_pos == emp::Point2D(1,1));
      CHECK(evaluator.Sense(state) == state.forward_cue); // Start tile gives forward cue
      state.cur_pos.Set(1,2);
      CHECK(state.cur_pos == emp::Point2D(1,2));
      CHECK(evaluator.Sense(state) == state.forward_cue); // Forward tile 
      state.cur_pos.Set(1,9);
      CHECK(state.cur_pos == emp::Point2D(1,9));
      CHECK(evaluator.Sense(state) == state.right_cue); // Right tile 
      state.cur_pos.Set(3,7);
      CHECK(state.cur_pos == emp::Point2D(3,7));
      CHECK(evaluator.Sense(state) == state.left_cue); // Left tile 
      state.cur_pos.Set(5,5);
      CHECK(state.cur_pos == emp::Point2D(5,5));
      CHECK(evaluator.Sense(state) == state.forward_cue); // Finish tile gives forward cue
      state.cur_pos.Set(0,0);
      CHECK(state.cur_pos == emp::Point2D(0,0));
      CHECK(evaluator.Sense(state) == state.empty_cue); // Empty tile
    }
    { // Random cues
      mabe::PathFollowEvaluator evaluator = mabe::PathFollowEvaluator();
      evaluator.LoadMap("path_follow_files/test_map_turns.txt");
      emp::Random local_random(12);
      evaluator.InitializeRandom(local_random, true);
      mabe::PathFollowState state;
      evaluator.InitializeState(state);
      CHECK(state.forward_cue == 4023234300);
      CHECK(state.right_cue == 2760829176);
      CHECK(state.left_cue == 843929936);
      CHECK(state.empty_cue == 136113612);
      CHECK(state.cur_pos == emp::Point2D(1,1));
      CHECK(evaluator.Sense(state) == state.forward_cue); // Start tile gives forward cue
      state.cur_pos.Set(1,2);
      CHECK(state.cur_pos == emp::Point2D(1,2));
      CHECK(evaluator.Sense(state) == state.forward_cue); // Forward tile 
      state.cur_pos.Set(1,9);
      CHECK(state.cur_pos == emp::Point2D(1,9));
      CHECK(evaluator.Sense(state) == state.right_cue); // Right tile 
      state.cur_pos.Set(3,7);
      CHECK(state.cur_pos == emp::Point2D(3,7));
      CHECK(evaluator.Sense(state) == state.left_cue); // Left tile 
      state.cur_pos.Set(5,5);
      CHECK(state.cur_pos == emp::Point2D(5,5));
      CHECK(evaluator.Sense(state) == state.forward_cue); // Finish tile gives forward cue
      state.cur_pos.Set(0,0);
      CHECK(state.cur_pos == emp::Point2D(0,0));
      CHECK(evaluator.Sense(state) == state.empty_cue); // Empty tile
    }
  }
}
