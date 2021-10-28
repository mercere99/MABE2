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

#include "emp/io/File.hpp"
#include "emp/datastructs/Matrix.hpp"
#include "emp/math/MVector.hpp"

namespace mabe {

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
      for(size_t row_idx = 0; row_idx < num_rows; ++row_idx){
        std::string & cur_line = file[row_idx];
        for(size_t col_idx = 0; col_idx < cur_line.size(); ++col_idx){
          std::cout << cur_line[col_idx]; 
          Tile cur_tile = Tile::OUT_OF_BOUNDS;
          switch(cur_line[col_idx]){
            case '.':
              cur_tile = Tile::RIGHT;
              break;
            case '+':
              cur_tile = Tile::FORWARD;
              break;
            case 'L':
              cur_tile = Tile::LEFT;
              break;
            case 'R':
              cur_tile = Tile::RIGHT;
              break;
            case 'X':
              cur_tile = Tile::FINISH;
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
      std::cout << "Map is " << num_rows << "x" << num_cols << "!" << std::endl;
    }

    void LoadAllMaps(const std::string& map_filenames_str){
      emp::vector<std::string> map_filename_vec;
      emp::slice(map_filenames_str, map_filename_vec, ';');
      for(auto filename : map_filename_vec){
        LoadMap(filename);
      }
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

    std::string input_trait = "input";          ///< Name of trait to put input values.
    std::string output_trait = "output";        ///< Name of trait to find output values.
    std::string score_trait = "score";         ///< Name of trait for organism performance
    std::string map_filenames="";               ///< ;-separated list map filenames to load.
    PathFollowEvaluator evaluator;

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
      LinkVar(input_trait, "input_trait", "Into which trait should input values be placed?");
      LinkVar(output_trait, "output_trait", "Out of which trait should output values be read?");
      LinkVar(score_trait, "score_trait", "Which trait stores path following performance?");
      LinkVar(map_filenames, "map_filenames", 
          "List of map files to load, separated by semicolons(;)");
    }

    void SetupModule() override {
      AddOwnedTrait<emp::vector<double>>(input_trait, "Input values (current board state)", emp::vector<double>({0.0}));
      AddRequiredTrait<emp::vector<double>>(output_trait); // Output values (move to make)
      AddOwnedTrait<double>(score_trait, "Path following score", 0.0);
      evaluator.LoadAllMaps(map_filenames);
    }


    PathFollowEvaluator::path_follow_decision_func_t ToOrgFunc(mabe::Organism & org) {
      return [this, &org](size_t cur_tile) {
        // Setup the hardware with proper inputs.
        org.GetTrait<emp::vector<double>>(input_trait) = {cur_tile};

        // Run the code.
        org.GenerateOutput();

        emp::vector<double> results = org.GetTrait<emp::vector<double>>(output_trait);

        // Determine the chosen move.
        size_t best_move = 0;
        const size_t move_cap = std::min<size_t>(results.size(), 4);
        for (size_t i = 1; i < move_cap; i++) {
          if (results[best_move] < results[i]) { best_move = i; }
        }
        return best_move;
      };
    }

    void OnUpdate(size_t ud) override {
      control.Verbose("UD ", ud, ": Running EvalPathFollow::OnUpdate()");

      emp_assert(control.GetNumPopulations() >= 1);
      // Loop through the living organisms in the target collection to evaluate each.
      mabe::Collection alive_collect( target_collect.GetAlive() );

      control.Verbose(" - ", alive_collect.GetSize(), " organisms found.");

      size_t org_id = 0;
      for (Organism & org : alive_collect) {
        double & score = org.GetTrait<double>(score_trait);
        auto org_func = ToOrgFunc(org);
        score = evaluator.EvalTrial(org_func, 0);
        std::cout << org_id << " ";
        org_id++;
      }
      std::cout << std::endl;
    }
  };

  MABE_REGISTER_MODULE(EvalPathFollow, "Evaluate organisms on their ability to follow a path.");
}

#endif
