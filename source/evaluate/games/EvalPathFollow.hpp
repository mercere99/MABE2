/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  EvalPathFollow.hpp
 *  @brief MABE Evaluation module that places organisms on the start of a nutrient-cued path 
 *      and rewards them for following it successfully.
 *
 *  TODO:
 *    - Make tile symbols configurable
 *    - Make max steps configurable 
 *    - Error checking
 *      - When loading map, it must have start and finish
 *      - When evaluating path, do some bounds checking on map_idx
 *    - Docstrings
 *    - Be careful about mixing initialization in place vs constructor
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

  /// \brief State of a single organism's progress on the path following task
  struct PathFollowState{
    bool initialized = false;     ///< Flag indicating if this state has been initialized
    size_t cur_map_idx;           ///< Index of the map being traversed 
    emp::BitVector visited_tiles; ///< A mask showing which tiles have been previously visited
    emp::Point2D<int> cur_pos;    ///< Current position of the organism in the map's matrix
    double raw_score;             ///< Number of unique valid tiles visited minus the number of steps taken off the path (not unique)
    double normalized_score;      ///< Raw score divided by the length of the path
    emp::Point2D<int> cur_dir;    ///< (x,y) facing direction of the organism. Kept as a Point2D to allow easy math with turning and moving the cur_pos
    uint32_t empty_cue = 1;       ///< Value of empty cues for this state, potentially randomized depending on the configuration options
    uint32_t forward_cue = 2;     ///< Value of forward cues for this state, potentially randomized depending on the configuration options
    uint32_t left_cue = 3;        ///< Value of left turn cues for this state, potentially randomized depending on the configuration options
    uint32_t right_cue = 4;       ///< Value of right turn cues for this state, potentially randomized depending on the configuration options

    PathFollowState() : cur_pos(-1,-1) { ; }
  };

  /// \brief Information of a single path that was loaded from file
  struct PathData{
    emp::StateGrid tile_map;     ///< The tile data of the path and surrounding emptiness 
    emp::Point2D<int> start_pos; ///< (x,y) coordinate of the starting tile
    emp::Point2D<int> start_dir; ///< New orgs will be initialized with this facing vector
    size_t path_length;          ///< Number of good ("path") tiles in this map 

    PathData(emp::StateGrid& _tile_map, emp::Point2D<int> _start_pos, 
        emp::Point2D<int> _start_dir, size_t _path_length) 
        : tile_map(_tile_map)
        , start_pos(_start_pos)
        , start_dir(_start_dir) 
        , path_length(_path_length) 
      { ; }
  };

  /// \brief Contains all information for multiple paths and can evaluate organisms on them
  struct PathFollowEvaluator{
    /// \brief A single tile in a tile map
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

    emp::vector<PathData> path_data_vec; ///< All the relevant data for each map loaded
    //size_t max_steps = 100; 
    emp::Random& rand;          ///< Reference to the main random number generator of MABE
    bool randomize_cues = true; ///< If true, each org receives random values for each type for cue(consistent through lifetime). Otherwise, cues have same values for all orgs
    
    protected: // Only a couple methods that should only be called internally

    /// Fetch the reward value for organism's current position
    ///
    /// Off path: -1
    /// On new tile of path: +1
    /// On previously-visited tile of path: 0
    double GetCurrentPosScore(const PathFollowState& state) const{
      // If we're off the path, decrement score
      int tile_id = path_data_vec[state.cur_map_idx].tile_map.GetState(state.cur_pos.GetX(), state.cur_pos.GetY());
      if(tile_id == Tile::EMPTY) return -1;
      // On a new tile of the path, add score (forward, left, right, finish)
      else if(!state.visited_tiles[
          (state.cur_pos.GetY() * path_data_vec[state.cur_map_idx].tile_map.GetWidth()) + 
           state.cur_pos.GetX()
      ]){
        return 1;
      }
      return 0; // Otherwise we've seen this tile of the path before, do nothing
    }

    /// Record the organism's current position as visited
    void MarkVisited(PathFollowState& state){
      state.visited_tiles.Set(
          (state.cur_pos.GetY() * path_data_vec[state.cur_map_idx].tile_map.GetWidth()) + state.cur_pos.GetX(), 
          true);
    }

    public: 
    PathFollowEvaluator(emp::Random& _rand) : rand(_rand) { ; } 

    /// Load a single map for the path following task
    template <typename... Ts>
    void LoadMap(Ts &&... args){
      // Set up the possible tile types for the grid
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
      // Load data
      grid.Load(std::forward<Ts>(args)...);
      // Extract data from each tile and store
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
      emp_assert(start_pos.GetX() != -1 && start_pos.GetY() != -1, 
          "Map must contain START tile!"); 
      emp_assert(start_dir.GetX() != 0 || start_dir.GetY() != 0, 
          "Map must contain START tile!"); 
      // Stash all data in one place
      path_data_vec.emplace_back(grid, start_pos, start_dir, path_length);
      std::cout << "Map #" << (path_data_vec.size() - 1) << " is " 
        << grid.GetWidth() << "x" << grid.GetHeight() << ", with " 
        << path_length << " path tiles!" << std::endl;
    }

    /// Load a semi-colon-separated list of maps from disk
    void LoadAllMaps(const std::string& map_filenames_str){
      emp::vector<std::string> map_filename_vec;
      emp::slice(map_filenames_str, map_filename_vec, ';');
      for(auto filename : map_filename_vec){
        LoadMap(filename);
      }
    }
    
    /// Initialize all properties of a PathFollowState to prepare it for the path follow task
    void InitializeState(PathFollowState& state, bool reset_map = true){
      state.initialized = true;
      if(reset_map) state.cur_map_idx = rand.GetUInt(path_data_vec.size());;
      emp_assert(path_data_vec.size() > state.cur_map_idx, "Cannot initialize state before loading the map!");
      state.visited_tiles.Resize(path_data_vec[state.cur_map_idx].tile_map.GetSize()); 
      state.visited_tiles.Clear();
      state.cur_pos = path_data_vec[state.cur_map_idx].start_pos;
      state.cur_dir = path_data_vec[state.cur_map_idx].start_dir;
      state.raw_score = 0;
      state.normalized_score = 0;
      if(randomize_cues){
        state.forward_cue = rand.GetUInt();
        state.right_cue = rand.GetUInt();
        while(state.right_cue == state.forward_cue){
          state.right_cue = rand.GetUInt();
        }
        state.left_cue = rand.GetUInt();
        while(state.left_cue == state.forward_cue || state.left_cue == state.right_cue){
          state.left_cue = rand.GetUInt();
        }
        state.empty_cue = rand.GetUInt();
        while(state.empty_cue == state.forward_cue || 
            state.empty_cue == state.right_cue || 
            state.empty_cue == state.left_cue){
          state.empty_cue = rand.GetUInt();
        }
      }
    }


    /// Move the organism in the direction it is facing then update and return score
    double Move(PathFollowState& state, int scale_factor = 1){
      if(!state.initialized) InitializeState(state);
      emp::Point2D<int> new_pos = state.cur_pos + (state.cur_dir * scale_factor);
      // Bounds check
      if(new_pos.GetX() < 0 || new_pos.GetX() >= path_data_vec[state.cur_map_idx].tile_map.GetWidth() ||
         new_pos.GetY() < 0 || new_pos.GetY() >= path_data_vec[state.cur_map_idx].tile_map.GetHeight()){
        new_pos = state.cur_pos;
      }
      if(state.cur_pos == new_pos) return state.normalized_score;
      state.cur_pos = new_pos;
      double score = GetCurrentPosScore(state);
      MarkVisited(state);
      state.raw_score += score;
      state.normalized_score = state.raw_score / path_data_vec[state.cur_map_idx].path_length;
      return state.normalized_score;
    }
    
    /// Rotate the organism clockwise by 90 degrees
    void RotateRight(PathFollowState& state){
      if(!state.initialized) InitializeState(state);
      state.cur_dir = state.cur_dir.GetRot90();
    }

    /// Rotate the organism counterclockwise by 90 degrees
    void RotateLeft(PathFollowState& state){
      if(!state.initialized) InitializeState(state);
      state.cur_dir = state.cur_dir.GetRot270();
    }

    /// Fetch the cue value of the tile the organism is currently on
    //
    // Note: While it sounds like this should be a const method, it is possible this is the
    //  organism's first interaction with the path, so we may need to initialize it
    uint32_t Sense(PathFollowState& state) { 
      if(!state.initialized) InitializeState(state);
      switch(path_data_vec[state.cur_map_idx].tile_map.GetState(state.cur_pos.GetX(), state.cur_pos.GetY())){
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
  };

  /// \brief MABE module that evaluates Avida-esque organisms on how well than can navigate a nutrient-cued path
  class EvalPathFollow : public Module {
    using inst_func_t = VirtualCPUOrg::inst_func_t;

  private:
    std::string score_trait = "score"; ///< Name of trait for organism performance
    std::string state_trait ="state";  ///< Name of trait that stores the path follow state
    std::string map_filenames="";      ///< ;-separated list map filenames to load.
    PathFollowEvaluator evaluator;     ///< The evaluator that does all of the actually computing and bookkeeping for the path follow task
    int pop_id = 0;                 ///< ID of the population to evaluate (and provide instructions to)

  public:
    EvalPathFollow(mabe::MABE & control,
                const std::string & name="EvalPathFollow",
                const std::string & desc="Evaluate organisms by how well they can follow a path.")
      : Module(control, name, desc)
      , evaluator(control.GetRandom())
    {
      SetEvaluateMod(true);
    }
    ~EvalPathFollow() { }

    /// Set up variables for configuration script
    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", "Population to evaluate.");
      LinkVar(score_trait, "score_trait", "Which trait stores path following performance?");
      LinkVar(state_trait, "state_trait", "Which trait stores organisms' path follow state?");
      LinkVar(map_filenames, "map_filenames", 
          "List of map files to load, separated by semicolons(;)");
      LinkVar(evaluator.randomize_cues, "randomize_cues", "If true, cues are assigned random values in for "
          "each new path");
    }
    
    /// Set up organism traits, load maps, and provide instructions to organisms
    void SetupModule() override {
      AddSharedTrait<double>(score_trait, "Path following score", 0.0);
      AddOwnedTrait<PathFollowState>(state_trait, "Organism's path follow state", { }); 
      evaluator.LoadAllMaps(map_filenames);
      SetupInstructions();
    }

    /// Package path following actions (e.g., move, turn) into instructions and provide them to the organisms via ActionMap
    void SetupInstructions(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      { // Move
        inst_func_t func_move = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          double score = evaluator.Move(hw.GetTrait<PathFollowState>(state_trait));
          hw.SetTrait<double>(score_trait, score);
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>("sg-move", func_move);
        action.data.AddVar<int>("inst_id", 27);
      }
      { // Move backward
        inst_func_t func_move_back = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          double score = evaluator.Move(hw.GetTrait<PathFollowState>(state_trait), -1);
          hw.SetTrait<double>(score_trait, score);
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-move-back", func_move_back);
        action.data.AddVar<int>("inst_id", 28);
      }
      { // Rotate right 
        inst_func_t func_rotate_right = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          evaluator.RotateRight(hw.GetTrait<PathFollowState>(state_trait));
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-rotate-r", func_rotate_right);
        action.data.AddVar<int>("inst_id", 29);
      }
      { // Rotate left 
        inst_func_t func_rotate_left = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
          evaluator.RotateLeft(hw.GetTrait<PathFollowState>(state_trait));
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-rotate-l", func_rotate_left);
        action.data.AddVar<int>("inst_id", 30);
      }
      { // Sense 
        inst_func_t func_sense = [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
          uint32_t val = evaluator.Sense(hw.GetTrait<PathFollowState>(state_trait));
          size_t reg_idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          hw.regs[reg_idx] = val;
        };
        Action& action = action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-sense", func_sense);
        action.data.AddVar<int>("inst_id", 31);
      }
    }
  };

  MABE_REGISTER_MODULE(EvalPathFollow, "Evaluate organisms on their ability to follow a path.");
}

#endif