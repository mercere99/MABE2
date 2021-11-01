/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  EvalPathFollow.hpp
 *  @brief MABE Evaluation module that places organisms on the start of a path, and rewards 
 *      them for following it successfully.
 *
 *  TODO:
 *    - Make tile symbols configurable
 *    - Make max steps configurable 
 *    - Error checking
 *      - When loading map, it must have start and finish
 *      - When evaluating path, do some bounds checking on map_idx
 */

#ifndef MABE_EVAL_PATH_FOLLOW_HPP
#define MABE_EVAL_PATH_FOLLOW_HPP

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"

#include "emp/io/File.hpp"
#include "emp/datastructs/Matrix.hpp"
#include "emp/math/MVector.hpp"

namespace mabe {

  struct PathFollowState{
  public:
    bool initialized = false;
    size_t cur_map_idx;
    emp::Matrix<bool> visited_tiles;
    emp::MVector cur_pos;
    double raw_score;
    double normalized_score;
    emp::MVector cur_dir;
    uint32_t forward_val = 12;
    uint32_t right_val = 14005;
    uint32_t left_val = 199;
    uint32_t empty_val = 27272;

    PathFollowState() : visited_tiles(0,0) { ; }
  };

  class PathFollowEvaluator{
  protected:
    enum Tile{
      EMPTY=0,
      FORWARD,
      LEFT,
      RIGHT,
      START,
      FINISH,
      OUT_OF_BOUNDS
    };
    
    emp::vector<emp::Matrix<Tile>> tile_map_vec;
    emp::vector<emp::MVector> start_pos_vec;
    emp::vector<emp::MVector> start_dir_vec;
    emp::vector<size_t> path_length_vec; 
    size_t max_steps = 100;
  public: 
    using path_follow_decision_func_t = std::function< size_t(size_t tile) >;
    void LoadMap(const std::string& filename){
      std::cout << "Loading map: " << filename << std::endl;
      emp::File file(filename);
      file.RemoveWhitespace();
      size_t num_rows = file.GetNumLines();
      size_t num_cols = file[0].size();
      emp::Matrix<Tile> tile_map(num_rows, num_cols);
      emp::MVector start_pos({-1,-1});
      emp::MVector start_dir({0,0});
      size_t path_length = 0;
      for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
        std::string & cur_line = file[row_idx];
        for(size_t col_idx = 0; col_idx < cur_line.size(); ++col_idx){
          std::cout << cur_line[col_idx]; 
          Tile cur_tile = Tile::OUT_OF_BOUNDS;
          switch(cur_line[col_idx]){
            case '.':
              cur_tile = Tile::EMPTY;
              break;
            case '+':
              cur_tile = Tile::FORWARD;
              path_length++;
              break;
            case 'L':
              cur_tile = Tile::LEFT;
              path_length++;
              break;
            case 'R':
              cur_tile = Tile::RIGHT;
              path_length++;
              break;
            case 'X':
              cur_tile = Tile::FINISH;
              path_length++;
              break;
            case '^':
              cur_tile = Tile::START;
              start_pos[0] = col_idx;
              start_pos[1] = row_idx;
              start_dir[0] = 0;
              start_dir[1] = -1;
              break;
            case 'v':
              cur_tile = Tile::START;
              start_pos[0] = col_idx;
              start_pos[1] = row_idx;
              start_dir[0] = 0;
              start_dir[1] = 1;
              break;
            case '>':
              cur_tile = Tile::START;
              start_pos[0] = col_idx;
              start_pos[1] = row_idx;
              start_dir[0] = 1;
              start_dir[1] = 0;
              break;
            case '<':
              cur_tile = Tile::START;
              start_pos[0] = col_idx;
              start_pos[1] = row_idx;
              start_dir[0] = -1;
              start_dir[1] = 0;
              break;
            default:
              emp_error("Unexpected symbol in path following map: ", cur_line[col_idx], 
                  " from file: ", filename);
          }
          tile_map[row_idx][col_idx] = cur_tile;
        }
        std::cout << std::endl;
      }
      tile_map_vec.push_back(tile_map);
      emp_assert(start_pos[0] != -1 && start_pos[1] != -1, "Map must contain START tile!"); 
      start_pos_vec.push_back(start_pos);
      emp_assert(start_dir[0] != 0 || start_dir[1] != 0, "Map must contain START tile!"); 
      start_dir_vec.push_back(start_dir);
      path_length_vec.push_back(path_length);
      std::cout << "Map is " << num_rows << "x" << num_cols << ", with " << 
          path_length << " tiles!" << std::endl;
    }

    void LoadAllMaps(const std::string& map_filenames_str){
      emp::vector<std::string> map_filename_vec;
      emp::slice(map_filenames_str, map_filename_vec, ';');
      for(auto filename : map_filename_vec){
        LoadMap(filename);
      }
    }
    
    void InitializeState(PathFollowState& state){
      state.initialized = true;
      state.cur_map_idx = 0;
      state.visited_tiles.ExpandTo(
          tile_map_vec[state.cur_map_idx].num_rows(), 
          tile_map_vec[state.cur_map_idx].num_cols(), false);
      state.cur_pos = start_pos_vec[state.cur_map_idx];
      state.cur_dir = start_dir_vec[state.cur_map_idx];
      state.raw_score = 0;
      state.normalized_score = 0;
    }

    double GetCurrentPosScore(PathFollowState& state){
      // If we're off the path, decrement score
      if(tile_map_vec[state.cur_map_idx].Get(state.cur_pos[1], state.cur_pos[0]) == Tile::EMPTY)
        return -1;
      // On a new tile of the path, add score
      if(!state.visited_tiles.Get(state.cur_pos[1], state.cur_pos[0])){
        state.visited_tiles.Set(state.cur_pos[1], state.cur_pos[0], true);
        return 1;
      }
      return 0; // Otherwise we've seen this tile of the path before, do nothing
    }

    double EvalMove(PathFollowState& state, double scale_factor = 1){
      if(!state.initialized) InitializeState(state);
      emp::MVector new_pos = state.cur_pos + (state.cur_dir * scale_factor);
      // Bounds check
      if(   new_pos[0] < 0 || new_pos[0] >= tile_map_vec[state.cur_map_idx].num_rows() ||
            new_pos[1] < 0 || new_pos[1] >= tile_map_vec[state.cur_map_idx].num_cols()){
        new_pos = state.cur_pos;
      }
      double score = 0;
      if(state.cur_pos != new_pos){
        score = GetCurrentPosScore(state);
      }
      state.cur_pos = new_pos;
      state.raw_score += score;
      if(state.raw_score > 0)
        state.normalized_score = state.raw_score / path_length_vec[state.cur_map_idx];
      else state.normalized_score = 0;
      //std::cout << state.cur_pos << " -> " << new_pos << "; " << 
      //    state.raw_score << " => "<< state.normalized_score << std::endl;
      return state.normalized_score;
    }
    
    void RotateRight(PathFollowState& state){
      if(!state.initialized) InitializeState(state);
        double tmp_val = state.cur_dir[0];
        state.cur_dir[0] = -1 * state.cur_dir[1];
        state.cur_dir[1] = tmp_val;
    }

    void RotateLeft(PathFollowState& state){
      if(!state.initialized) InitializeState(state);
        double tmp_val = state.cur_dir[0];
        state.cur_dir[0] = state.cur_dir[1];
        state.cur_dir[1] = -1 * tmp_val;
    }

    uint32_t Sense(PathFollowState& state){
      switch(tile_map_vec[state.cur_map_idx].Get(state.cur_pos[1], state.cur_pos[0])){
        case Tile::EMPTY:
          return state.empty_val;
          break;
        case Tile::LEFT:
          return state.left_val;
          break;
        case Tile::RIGHT:
          return state.right_val;
          break;
        case Tile::FORWARD:
          return state.forward_val;
          break;
        case Tile::START:
          return state.forward_val;
          break;
       default:
          return 0;
          break;
      }
      return 0;
    }

    double EvalTrial(path_follow_decision_func_t& decision_func, size_t map_idx, 
        bool verbose = false){
      emp::Matrix<Tile>& cur_map = tile_map_vec[map_idx];
      emp::Matrix<bool> visited_tiles(cur_map.num_cols(), cur_map.num_rows(), false);
      emp::MVector cur_pos = start_pos_vec[map_idx];
      emp::MVector cur_dir = start_dir_vec[map_idx];
      size_t cur_step = 0;
      double score = 0;
      bool finished = false;
      while(!finished && cur_step < max_steps){
        size_t cur_tile_val = (size_t)cur_map[cur_pos[0]][cur_pos[1]];
        size_t res = decision_func(cur_tile_val);
        switch(res){
          case 0:
            if (verbose) std::cout << "Do nothing" << std::endl;
            break;
          case 1:
            if (verbose) std::cout << "Forward" << std::endl;
            cur_pos += cur_dir;
            break;
          case 2:
            {
              if (verbose) std::cout << "Right" << std::endl;
              double tmp_val = cur_dir[0];
              cur_dir[0] = -1 * cur_dir[1];
              cur_dir[1] = tmp_val;
            }
            break;
          case 3:
            {
              if (verbose) std::cout << "Left" << std::endl;
              double tmp_val = cur_dir[0];
              cur_dir[0] = cur_dir[1];
              cur_dir[1] = -1 * tmp_val;
            }
            break;
        } 
        if(cur_pos[0] < 0 || cur_pos[1] < 0 || 
            cur_pos[0] >= cur_map.num_cols() || cur_pos[1] >= cur_map.num_rows()){
          score -= 100;
          break;
        }
        if(!visited_tiles[cur_pos[0]][cur_pos[1]]){
          visited_tiles[cur_pos[0]][cur_pos[1]] = true;
          Tile& new_tile = cur_map[cur_pos[0]][cur_pos[1]];
          switch(new_tile){
            case Tile::EMPTY:
              score -= 1;
              break;
            case Tile::FORWARD:
            case Tile::RIGHT:
            case Tile::LEFT:
              score += 2;
              break;
            case Tile::FINISH:
              score += 100;
              finished = true;
              break;
          }
        }
        cur_step++;
      } 
      return score; 
    }
  };


  class EvalPathFollow : public Module {

  private:
    Collection target_collect;                  ///< Which organisms should we evaluate?

    std::string score_trait = "score";         ///< Name of trait for organism performance
    std::string state_trait ="state";          ///< Name of trait that stores the path follow state
    std::string map_filenames="";              ///< ;-separated list map filenames to load.
    PathFollowEvaluator evaluator;
    size_t pop_id = 0;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_move;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_rotate_right;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_rotate_left;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_sense;
    std::function<void(VirtualCPUOrg&, const VirtualCPUOrg::inst_t&)> func_move_back;

  public:
    EvalPathFollow(mabe::MABE & control,
                const std::string & name="EvalPathFollow",
                const std::string & desc="Evaluate organisms by how well they can follow a path.")
      : Module(control, name, desc)
      , target_collect(control.GetPopulation(0))
    {
      SetEvaluateMod(true);
    }
    ~EvalPathFollow() { }

    void SetupConfig() override {
      LinkCollection(target_collect, "target", "Which population(s) should we evaluate?");
      LinkVar(score_trait, "score_trait", "Which trait stores path following performance?");
      LinkVar(state_trait, "state_trait", "Which trait stores organisms' path follow state?");
      LinkVar(map_filenames, "map_filenames", 
          "List of map files to load, separated by semicolons(;)");
    }

    void SetupInstructions(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      // Move
      {
        func_move = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          double score = evaluator.EvalMove(hw.GetTrait<PathFollowState>(state_trait));
          hw.SetTrait<double>(score_trait, score);
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-move", func_move);
        action.data.AddVar<int>("inst_id", 27);
      }
      // Move backward
      {
        func_move_back = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          double score = evaluator.EvalMove(hw.GetTrait<PathFollowState>(state_trait), -1);
          hw.SetTrait<double>(score_trait, score);
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-move-back", func_move_back);
        action.data.AddVar<int>("inst_id", 28);
      }
      // Rotate right 
      {
        func_rotate_right = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          evaluator.RotateRight(hw.GetTrait<PathFollowState>(state_trait));
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-rotate-r", func_rotate_right);
        action.data.AddVar<int>("inst_id", 29);
      }
      // Rotate left 
      {
        func_rotate_left = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          evaluator.RotateLeft(hw.GetTrait<PathFollowState>(state_trait));
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-rotate-l", func_rotate_left);
        action.data.AddVar<int>("inst_id", 30);
      }
      // Sense 
      {
        func_sense = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          uint32_t val = evaluator.Sense(hw.GetTrait<PathFollowState>(state_trait));
          size_t reg_idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          hw.regs[reg_idx] = val;
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-sense", func_sense);
        action.data.AddVar<int>("inst_id", 31);
      }
    }

    void SetupModule() override {
      AddSharedTrait<double>(score_trait, "Path following score", 0.0);
      AddOwnedTrait<PathFollowState>(state_trait, "Organism's path follow state", { }); 
      evaluator.LoadAllMaps(map_filenames);
      SetupInstructions();
    }
  };

  MABE_REGISTER_MODULE(EvalPathFollow, "Evaluate organisms on their ability to follow a path.");
}

#endif
