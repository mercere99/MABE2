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
 *    - Make max steps configurable? 
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
#include "emp/bits/BitVector.hpp"

namespace mabe {

  /// \brief State of a single organism's progress on the path following task
  struct PathFollowState{
    bool initialized;             ///< Flag indicating if this state has been initialized
    size_t cur_map_idx;           ///< Index of the map being traversed 
    emp::BitVector visited_tiles; ///< A mask showing which tiles have been previously visited
    emp::StateGridStatus status;  ///< Stores position, direction, and interfaces with grid 
    double raw_score;             /**< Number of unique valid tiles visited minus the number
                                       of steps taken off the path (not unique) */
    uint32_t empty_cue;           /**< Value of empty cues for this state, potentially 
                                       randomized depending on the configuration options */
    uint32_t forward_cue;         /**< Value of forward cues for this state, potentially 
                                       randomized depending on the configuration options */
    uint32_t left_cue;            /**< Value of left turn cues for this state, potentially 
                                       randomized depending on the configuration options */
    uint32_t right_cue;           /**< Value of right turn cues for this state, potentially 
                                       randomized depending on the configuration options */

    PathFollowState(): initialized(false), cur_map_idx(0), visited_tiles(), status(),
        raw_score(0), empty_cue(1), forward_cue(2), left_cue(3), right_cue(4) { ; }
    
    PathFollowState(const PathFollowState&){ // Ignore copy, just prep to initialize
      raw_score = 0;
      initialized = false;
    }
    PathFollowState(PathFollowState&&){ // Ignore move, just prep to initialize
      raw_score = 0;
      initialized = false;
    }
    PathFollowState& operator=(PathFollowState&){ // Ignore copy, just prep to initialize
      raw_score = 0;
      initialized = false;
      return *this;
    }
    PathFollowState& operator=(PathFollowState&&){ // Ignore move, just prep to initialize
      raw_score = 0;
      initialized = false;
      return *this;
    }
  };

  /// \brief Information of a single path that was loaded from file
  struct PathData{
    emp::StateGrid grid;  ///< The tile data of the path and surrounding emptiness 
    size_t start_x;       ///< X coordinate of starting position
    size_t start_y;       ///< Y coordinate of starting position
    int start_facing;     /**< Facing direction for new organisms. 
                              0=UL, 1=Up, 2=UR, 3=Right, 4=DR, 5=Down, 6=DL, 
                              7=Left (+=Clockwise) Matches StateGridStatus */
    size_t path_length;   ///< Number of good ("path") tiles in this map 

    PathData() : 
      start_x(0), start_y(0), start_facing(0), path_length(0){;} 
    PathData(emp::StateGrid& _grid, size_t _start_x, size_t _start_y, 
        int _start_facing, size_t _path_length) 
        : grid(_grid)
        , start_x(_start_x)
        , start_y(_start_y)
        , start_facing(_start_facing)
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
    emp::Random& rand;          ///< Reference to the main random number generator of MABE
    bool randomize_cues; /**< If true, each org receives random values for each type for cue
                                  (consistent through lifetime). Otherwise, cues have same 
                                  values for all orgs */
    
    public: 
    PathFollowEvaluator(emp::Random& _rand) : path_data_vec(), rand(_rand), 
        randomize_cues(true) { ; } 

    /// Fetch the number of maps that are currently stored 
    size_t GetNumMaps(){ return path_data_vec.size(); }

    /// Divide raw score by the length of the current path
    double GetNormalizedScore(PathFollowState& state) const{
      return static_cast<double>(state.raw_score) / path_data_vec[state.cur_map_idx].path_length;
    }

    /// Load a single map for the path following task
    template <typename... Ts>
    void LoadMap(Ts &&... args){
      // Create our PathData to be filled
      path_data_vec.emplace_back();
      PathData& path_data = *(path_data_vec.rbegin()); 
      // Set up the possible tile types for the grid (we ignore the score value)
      path_data.grid.AddState(Tile::EMPTY,       '.', 1.0, "empty");
      path_data.grid.AddState(Tile::FORWARD,     '+', 1.0, "forward");
      path_data.grid.AddState(Tile::LEFT,        'L', 1.0, "turn_left");
      path_data.grid.AddState(Tile::RIGHT,       'R', 1.0, "turn_right");
      path_data.grid.AddState(Tile::FINISH,      'X', 1.0, "finish");
      path_data.grid.AddState(Tile::START_UP,    '^', 1.0, "start_up");
      path_data.grid.AddState(Tile::START_DOWN,  'v', 1.0, "start_down");
      path_data.grid.AddState(Tile::START_LEFT,  '<', 1.0, "start_left");
      path_data.grid.AddState(Tile::START_RIGHT, '>', 1.0, "start_right");
      // Load data
      path_data.grid.Load(std::forward<Ts>(args)...);
      // Extract data from each tile and store
      bool has_start = false;
      bool has_finish = false;
      for(size_t row_idx = 0; row_idx < path_data.grid.GetHeight(); ++row_idx){
        for(size_t col_idx = 0; col_idx < path_data.grid.GetWidth(); ++col_idx){
          switch(path_data.grid.GetState(col_idx, row_idx)){
            case Tile::EMPTY:
              break;
            case Tile::FORWARD:
              path_data.path_length++;
              break;
            case Tile::LEFT:
              path_data.path_length++;
              break;
            case Tile::RIGHT:
              path_data.path_length++;
              break;
            case Tile::FINISH:
              path_data.path_length++;
              has_finish = true;
              break;
            case Tile::START_UP: 
              path_data.start_x = col_idx;
              path_data.start_y = row_idx;
              path_data.start_facing = 1;
              has_start = true;
              break;
            case Tile::START_DOWN:
              path_data.start_x = col_idx;
              path_data.start_y = row_idx;
              path_data.start_facing = 5;
              has_start = true;
              break;
            case Tile::START_LEFT:
              path_data.start_x = col_idx;
              path_data.start_y = row_idx;
              path_data.start_facing = 7;
              has_start = true;
              break;
            case Tile::START_RIGHT:
              path_data.start_x = col_idx;
              path_data.start_y = row_idx;
              path_data.start_facing = 3;
              has_start = true;
              break;
          }
        }
      }
      if(!has_start){
        emp_error("Error! Map does not have a start tile!");
      }
      if(!has_finish){
        emp_error("Error! Map does not have a finish tile! (character: X)");
      }
      std::cout << "Map #" << (path_data_vec.size() - 1) << " is " 
        << path_data.grid.GetWidth() << "x" << path_data.grid.GetHeight() << ", with " 
        << path_data.path_length << " path tiles!" << std::endl;
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
      state.visited_tiles.Resize(path_data_vec[state.cur_map_idx].grid.GetSize()); 
      state.visited_tiles.Clear();
      state.status.Set(
        path_data_vec[state.cur_map_idx].start_x,
        path_data_vec[state.cur_map_idx].start_y,
        path_data_vec[state.cur_map_idx].start_facing
      );
      state.raw_score = 0;
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
    
    /// Fetch the data of the state's current path
    PathData& GetCurPath(const PathFollowState& state){
      return path_data_vec[state.cur_map_idx];
    }
    
    /// Fetch the data of the state's current path
    const PathData& GetCurPath(const PathFollowState& state) const{
      return path_data_vec[state.cur_map_idx];
    }

    /// Record the organism's current position as visited
    void MarkVisited(PathFollowState& state){
      state.visited_tiles.Set(state.status.GetIndex(GetCurPath(state).grid), true);
    }

    /// Fetch the reward value for organism's current position
    ///
    /// Off path: -1
    /// On new tile of path: +1
    /// On previously-visited tile of path: 0
    double GetCurrentPosScore(const PathFollowState& state) const{
      // If we're off the path, decrement score
      int tile_id = state.status.Scan(GetCurPath(state).grid);
      if(tile_id == Tile::EMPTY) return -1;
      // On a new tile of the path, add score (forward, left, right, finish)
      else if(!state.visited_tiles[ state.status.GetIndex(GetCurPath(state).grid) ]) return 1;
      return 0; // Otherwise we've seen this tile of the path before, do nothing
    }

    /// Move the organism in the direction it is facing, then update and return score
    double Move(PathFollowState& state, int scale_factor = 1){
      if(!state.initialized) InitializeState(state);
      state.status.Move(GetCurPath(state).grid, scale_factor);
      double score = GetCurrentPosScore(state);
      MarkVisited(state);
      state.raw_score += score;
      return GetNormalizedScore(state);
    }
    
    /// Rotate the organism clockwise by 90 degrees
    void RotateRight(PathFollowState& state){
      if(!state.initialized) InitializeState(state);
      state.status.Rotate(2);
    }

    /// Rotate the organism counterclockwise by 90 degrees
    void RotateLeft(PathFollowState& state){
      if(!state.initialized) InitializeState(state);
      state.status.Rotate(-2);
    }

    /// Fetch the cue value of the tile the organism is currently on
    //
    // Note: While it sounds like this should be a const method, it is possible this is the
    //  organism's first interaction with the path, so we may need to initialize it
    uint32_t Sense(PathFollowState& state) { 
      if(!state.initialized) InitializeState(state);
      switch(state.status.Scan(GetCurPath(state).grid)){
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
          return state.forward_cue;
          break;
        case Tile::START_RIGHT:
          return state.forward_cue;
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

  /** \brief MABE module that evaluates Avida-esque organisms on how well they can 
             navigate a nutrient-cued path*/
  class EvalPathFollow : public Module {
    using inst_func_t = VirtualCPUOrg::inst_func_t;

  private:
    std::string score_trait = "score"; ///< Name of trait for organism performance
    std::string state_trait ="state";  ///< Name of trait that stores the path follow state
    std::string map_filenames="";      ///< ;-separated list map filenames to load.
    PathFollowEvaluator evaluator;     /**< The evaluator that does all of the actually 
                                            computing and bookkeeping for the path follow 
                                            task */
    int pop_id = 0;              /**< ID of the population to evaluate 
                                         (and provide instructions to) */

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
      LinkPop(pop_id, "target_pop", 
          "Population to evaluate.");
      LinkVar(score_trait, "score_trait", 
          "Which trait stores path following performance?");
      LinkVar(state_trait, "state_trait", 
          "Which trait stores organisms' path follow state?");
      LinkVar(map_filenames, "map_filenames", 
          "List of map files to load, separated by semicolons(;)");
      LinkVar(evaluator.randomize_cues, "randomize_cues", 
          "If true, cues are assigned random values in for each new path");
    }
    
    /// Set up organism traits, load maps, and provide instructions to organisms
    void SetupModule() override {
      AddSharedTrait<double>(score_trait, "Path following score", 0.0);
      AddOwnedTrait<PathFollowState>(state_trait, "Organism's path follow state", { }); 
      evaluator.LoadAllMaps(map_filenames);
      SetupInstructions();
    }

    /// Package path following actions (e.g., move, turn) into instructions and provide 
    /// them to the organisms via ActionMap
    void SetupInstructions(){
      ActionMap& action_map = control.GetActionMap(pop_id);
      { // Move
        inst_func_t func_move = 
          [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
            double score = evaluator.Move(hw.GetTrait<PathFollowState>(state_trait));
            hw.SetTrait<double>(score_trait, score);
          };
        action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-move", func_move);
      }
      { // Move backward
        inst_func_t func_move_back = 
          [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
            double score = evaluator.Move(hw.GetTrait<PathFollowState>(state_trait), -1);
            hw.SetTrait<double>(score_trait, score);
          };
        action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-move-back", func_move_back);
      }
      { // Rotate right 
        inst_func_t func_rotate_right = 
          [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
            evaluator.RotateRight(hw.GetTrait<PathFollowState>(state_trait));
          };
        action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-rotate-r", func_rotate_right);
      }
      { // Rotate left 
        inst_func_t func_rotate_left = 
          [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& /*inst*/){
            evaluator.RotateLeft(hw.GetTrait<PathFollowState>(state_trait));
          };
        action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-rotate-l", func_rotate_left);
      }
      { // Sense 
        inst_func_t func_sense = 
          [this](VirtualCPUOrg& hw, const VirtualCPUOrg::inst_t& inst){
            uint32_t val = evaluator.Sense(hw.GetTrait<PathFollowState>(state_trait));
            size_t reg_idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
            hw.regs[reg_idx] = val;
          };
        action_map.AddFunc<void, VirtualCPUOrg&, const VirtualCPUOrg::inst_t&>(
            "sg-sense", func_sense);
      }
    }
  };

  MABE_REGISTER_MODULE(EvalPathFollow, 
      "Evaluate organisms on their ability to follow a nutrient-cued path.");
}

#endif
