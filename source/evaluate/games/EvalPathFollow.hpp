/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
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
 *    - Docstrings
 */

#ifndef MABE_EVAL_PATH_FOLLOW_HPP
#define MABE_EVAL_PATH_FOLLOW_HPP

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"
#include "../../tools/StateGrid.hpp"

#include "emp/io/File.hpp"
#include "emp/geometry/Point2D.hpp"
#include "emp/bits/BitVector.hpp"

namespace mabe {

  struct PathFollowState{
  public:
    bool initialized = false;
    size_t cur_map_idx;
    emp::BitVector visited_tiles; 
    emp::Point2D<int> cur_pos;
    double raw_score; // TODO: Be careful about mixing initialization in place vs constructor
    double normalized_score;
    emp::Point2D<int> cur_dir;
    uint32_t empty_cue = 1;
    uint32_t forward_cue = 2;
    uint32_t left_cue = 3;
    uint32_t right_cue = 4; 

    PathFollowState() : cur_pos(-1,-1) { ; }
  };

  struct PathFollowEvaluator{
    enum Tile{
      EMPTY=0,
      FORWARD,
      LEFT,
      RIGHT,
      START_UP,
      START_DOWN,
      START_LEFT,
      START_RIGHT,
      FINISH,
      OUT_OF_BOUNDS
    };

    
    size_t num_maps = 0;
    emp::vector<emp::StateGrid> tile_map_vec; // For cleanliness, make a TileMap class and  
                                                // collapse these four vectors 
    emp::vector<emp::Point2D<int>> start_pos_vec;
    emp::vector<emp::Point2D<int>> start_dir_vec;
    emp::vector<size_t> path_length_vec; 
    size_t max_steps = 100;
    emp::Ptr<emp::Random> rand_ptr;
    bool randomize_cues = true;
    
    PathFollowEvaluator() : rand_ptr(nullptr) { ; } 

    template <typename... Ts>
    void LoadMap(Ts &&... args){
      emp::StateGrid grid;
      grid.AddState(Tile::EMPTY,       '.', 1.0, "empty");
      grid.AddState(Tile::FORWARD,     '+', 1.0, "forward");
      grid.AddState(Tile::LEFT,        'L', 1.0, "turn_left");
      grid.AddState(Tile::RIGHT,       'R', 1.0, "turn_right");
      grid.AddState(Tile::FINISH,      'X', 1.0, "finish");
      grid.AddState(Tile::START_UP,    '^', 1.0, "start_up");
      grid.AddState(Tile::START_DOWN,  'v', 1.0, "start_down");
      grid.AddState(Tile::START_LEFT,  '<', 1.0, "start_left");
      grid.AddState(Tile::START_RIGHT, '>', 1.0, "start_right");

      grid.Load(std::forward<Ts>(args)...);
      emp::Point2D<int> start_pos({-1,-1});
      emp::Point2D<int> start_dir({0,0});
      size_t path_length = 0;
      for(size_t row_idx = 0; row_idx < grid.GetHeight(); ++row_idx){
        for(size_t col_idx = 0; col_idx < grid.GetWidth(); ++col_idx){
          switch(grid.GetState(col_idx, row_idx)){
            case Tile::EMPTY:
              break;
            case Tile::FORWARD:
              path_length++;
              break;
            case Tile::LEFT:
              path_length++;
              break;
            case Tile::RIGHT:
              path_length++;
              break;
            case Tile::FINISH:
              path_length++;
              break;
            case Tile::START_UP: 
              start_pos.Set(col_idx, row_idx);
              start_dir.Set(0, -1);
              break;
            case Tile::START_DOWN:
              start_pos.Set(col_idx, row_idx);
              start_dir.Set(0, 1);
              break;
            case Tile::START_LEFT:
              start_pos.Set(col_idx, row_idx);
              start_dir.Set(-1, 0);
              break;
            case Tile::START_RIGHT:
              start_pos.Set(col_idx, row_idx);
              start_dir.Set(1, 0);
              break;
          }
        }
      }
      tile_map_vec.push_back(grid);
      emp_assert(start_pos.GetX() != -1 && start_pos.GetY() != -1, 
          "Map must contain START tile!"); 
      start_pos_vec.push_back(start_pos);
      emp_assert(start_dir.GetX() != 0 || start_dir.GetY() != 0, 
          "Map must contain START tile!"); 
      start_dir_vec.push_back(start_dir);
      path_length_vec.push_back(path_length);
      num_maps++;
      std::cout << "Map #" << (num_maps - 1) << " is " 
        << grid.GetWidth() << "x" << grid.GetHeight() << ", with " 
        << path_length << " path tiles!" << std::endl;
    }

    void LoadAllMaps(const std::string& map_filenames_str){
      emp::vector<std::string> map_filename_vec;
      emp::slice(map_filenames_str, map_filename_vec, ';');
      for(auto filename : map_filename_vec){
        LoadMap(filename);
      }
    }
    
    void InitializeState(PathFollowState& state, bool reset_map = true){
      state.initialized = true;
      if(reset_map) state.cur_map_idx = 0;
      emp_assert(num_maps > state.cur_map_idx, "Cannot initialize state before loading the map!");
      state.visited_tiles.Resize(tile_map_vec[state.cur_map_idx].GetSize()); 
      state.visited_tiles.Clear();
      state.cur_pos = start_pos_vec[state.cur_map_idx];
      state.cur_dir = start_dir_vec[state.cur_map_idx];
      state.raw_score = 0;
      state.normalized_score = 0;
      if(randomize_cues){
        emp_assert(rand_ptr.Raw() != nullptr, 
            "PathFollowEvaluator::InitializeRandom must be called before cues can be randomized!");
        state.forward_cue = rand_ptr->GetUInt();
        state.right_cue = rand_ptr->GetUInt();
        while(state.right_cue == state.forward_cue) state.right_cue = rand_ptr->GetUInt();
        state.left_cue = rand_ptr->GetUInt();
        while(state.left_cue == state.forward_cue || state.left_cue == state.right_cue)
            state.left_cue = rand_ptr->GetUInt();
        state.empty_cue = rand_ptr->GetUInt();
        while(state.empty_cue == state.forward_cue || 
            state.empty_cue == state.right_cue || 
            state.empty_cue == state.left_cue)
            state.empty_cue = rand_ptr->GetUInt();
      }
    }

    double GetCurrentPosScore(PathFollowState& state){ // Mark both arg and function const
      // If we're off the path, decrement score
      int tile_id = tile_map_vec[state.cur_map_idx].GetState(state.cur_pos.GetX(), state.cur_pos.GetY());
      if(tile_id == Tile::EMPTY) return -1;
      // On a new tile of the path, add score (forward, left, right, finish)
      else if(!state.visited_tiles[
          (state.cur_pos.GetY() * tile_map_vec[state.cur_map_idx].GetWidth()) + 
           state.cur_pos.GetX()
      ]){
        return 1;
      }
      return 0; // Otherwise we've seen this tile of the path before, do nothing
    }

    void MarkVisited(PathFollowState& state){
      state.visited_tiles.Set(
          (state.cur_pos.GetY() * tile_map_vec[state.cur_map_idx].GetWidth()) + state.cur_pos.GetX(), 
          true);
    }

    double Move(PathFollowState& state, int scale_factor = 1){
      if(!state.initialized) InitializeState(state);
      emp::Point2D<int> new_pos = state.cur_pos + (state.cur_dir * scale_factor);
      // Bounds check
      if(new_pos.GetX() < 0 || new_pos.GetX() >= tile_map_vec[state.cur_map_idx].GetWidth() ||
         new_pos.GetY() < 0 || new_pos.GetY() >= tile_map_vec[state.cur_map_idx].GetHeight()){
        new_pos = state.cur_pos;
      }
      if(state.cur_pos == new_pos) return state.normalized_score;
      state.cur_pos = new_pos;
      double score = GetCurrentPosScore(state);
      MarkVisited(state);
      state.raw_score += score;
      state.normalized_score = state.raw_score / path_length_vec[state.cur_map_idx];
      return state.normalized_score;
    }
    
    void RotateRight(PathFollowState& state){
      if(!state.initialized) InitializeState(state);
        state.cur_dir = state.cur_dir.GetRot90();
    }

    void RotateLeft(PathFollowState& state){
      if(!state.initialized) InitializeState(state);
        state.cur_dir = state.cur_dir.GetRot270();
    }

    uint32_t Sense(PathFollowState& state){ // Could be const
      if(!state.initialized) InitializeState(state);
      switch(tile_map_vec[state.cur_map_idx].GetState(state.cur_pos.GetX(), state.cur_pos.GetY())){
        case Tile::EMPTY:
          return state.empty_cue;
          break;
        case Tile::LEFT:
          return state.left_cue;
          break;
        case Tile::RIGHT:
          return state.right_cue;
          break;
        case Tile::FORWARD:
          return state.forward_cue;
          break;
        case Tile::START_UP:
          return state.forward_cue;
          break;
        case Tile::START_DOWN:
          return state.forward_cue;
          break;
        case Tile::START_LEFT:
          return state.left_cue;
          break;
        case Tile::START_RIGHT:
          return state.right_cue;
          break;
        case Tile::FINISH:
          return state.forward_cue;
          break;
       default:
          return state.empty_cue;
          break;
      }
      return state.empty_cue;
    }

    void InitializeRandom(emp::Random& _rand, bool _randomize_cues){
      rand_ptr = &_rand;
      randomize_cues = _randomize_cues;
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
    bool randomize_cues = true;
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
      LinkVar(randomize_cues, "randomize_cues", "If true, cues are assigned random values in for "
          "each new path");
    }

    void SetupInstructions(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      // TODO: make std::functions auto and then forget about them
      { // Move
        func_move = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          double score = evaluator.Move(hw.GetTrait<PathFollowState>(state_trait));
          hw.SetTrait<double>(score_trait, score);
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-move", func_move);
        action.data.AddVar<int>("inst_id", 27);
      }
      { // Move backward
        func_move_back = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          double score = evaluator.Move(hw.GetTrait<PathFollowState>(state_trait), -1);
          hw.SetTrait<double>(score_trait, score);
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-move-back", func_move_back);
        action.data.AddVar<int>("inst_id", 28);
      }
      { // Rotate right 
        func_rotate_right = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          evaluator.RotateRight(hw.GetTrait<PathFollowState>(state_trait));
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-rotate-r", func_rotate_right);
        action.data.AddVar<int>("inst_id", 29);
      }
      { // Rotate left 
        func_rotate_left = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          evaluator.RotateLeft(hw.GetTrait<PathFollowState>(state_trait));
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-rotate-l", func_rotate_left);
        action.data.AddVar<int>("inst_id", 30);
      }
      { // Sense 
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
      evaluator.InitializeRandom(control.GetRandom(), randomize_cues);
      SetupInstructions();
    }
  };

  MABE_REGISTER_MODULE(EvalPathFollow, "Evaluate organisms on their ability to follow a path.");
}

#endif
