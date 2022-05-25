/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2022.
 *
 *  @file  EvalDoors.hpp
 *  @brief MABE Evaluation module that places the organism in a room with N+1 doors. 
 *          In each room, a symbol indicates which door is the correct one. 
 *          The rooms are configurable such that the symbol is either set or random between 
 *          trials. 
 *          The extra door is the "exit" door. If the wrong door is chosen, the next room
 *          shows the "wrong choice" symbol, and organisms should then return to the 
 *          previous room via the exit door.
 * 
 * Notes: 
 *    - If an organism has taken a wrong door and now should take the exit, we say they are in
 *        an "exit room"
 *      - Otherwise, they are in a "door room"
 *
 */

#ifndef MABE_EVAL_DOORS_HPP
#define MABE_EVAL_DOORS_HPP

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../orgs/VirtualCPUOrg.hpp"

#include "emp/bits/BitVector.hpp"
#include "emp/io/File.hpp"
#include "emp/tools/string_utils.hpp"

namespace mabe {
    
  /// \brief A collection of all the trait names used in EvalDoors
  struct EvalDoors_TraitNames{
    std::string score_trait = "score"; ///< Name of trait for organism performance
    std::string accuracy_trait = "accuracy"; ///< Name of trait for organism accuracy
    std::string state_trait ="state";  ///< Name of trait that stores the task state
    std::string door_rooms_trait ="door_rooms"; /**< Name of trait that stores the number of
                                                     "door rooms" visited */
    std::string exit_rooms_trait ="exit_rooms"; /**< Name of trait that stores the number of
                                                     "exit rooms" visited */
    std::string correct_doors_trait ="correct_doors"; /**< Name of trait that stores the 
                                                           number of doors correctly taken*/
    std::string incorrect_doors_trait ="incorrect_doors"; /**< Name of trait that stores the
                                                           number of doors incorrectly
                                                           taken*/
    std::string correct_exits_trait ="correct_exits"; /**< Name of trait that stores the 
                                                           number of exits correctly taken*/
    std::string incorrect_exits_trait ="incorrect_exits"; /**< Name of trait that stores the
                                                         number of exits incorrectly taken*/
  };

  /// \brief State of a single organism's progress on the doors task
  struct DoorsState{
    using data_t = uint32_t;

    bool initialized = false;     ///< Flag indicating if this state has been initialized
    emp::vector<data_t> prev_room_vec;   ///< Vector of cues from rooms previously visited
    emp::vector<data_t> door_choice_vec; ///< Vector of the doors the org has chosen
    double score = 0;             ///< Summarized score of the organism 
    emp::vector<data_t> cue_vec;  ///< Vector containing the value of each cue (random or not)
    data_t current_cue;           ///< Cue of the current room the organism is in
    size_t correct_doors_taken = 0;   ///< Number of times the org entered the correct door
    size_t incorrect_doors_taken = 0; ///< Number of times the org entered the wrong door
    size_t correct_exits_taken = 0;   /**< Number of times the org took the exit when it 
                                           should have */
    size_t incorrect_exits_taken = 0; /**< Number of times the org took the exit when it 
                                           should *not* have */
    size_t door_rooms_visited = 0; /// Number of "door" rooms the organism has visited
    size_t exit_rooms_visited = 0; /// Number of "exit" rooms the organism has visited

    DoorsState() { ; }
    DoorsState(const DoorsState&){ // Ignore copy, just reset
      initialized = false;
      score = 0;
    }
  };

  /// \brief Handles all evaluation of the doors task
  struct DoorsEvaluator{
    public:
    using org_t = VirtualCPUOrg;

    protected:
    emp::Random& rand;  ///< Reference to the main random number generator of MABE
    emp::BitVector random_cues_mask; ///< Bitmask for which cues are randomized (1s = random) 
    const size_t exit_cue_idx = 0; ///< Index of the exit in the cue vector 
    
    public: 
    DoorsEvaluator(emp::Random& _rand) : rand(_rand) { ; } 

    /// Fetch the number of doors in each room (includes exit)
    size_t GetNumDoors() const { return random_cues_mask.GetSize(); }

    /// Set the cues bitmask to the given mask
    void SetCuesMask(const emp::BitVector& mask){ random_cues_mask = mask; }
    
    /// Calculate the score for the given state
    double GetScore(const DoorsState& state) const{
      double score = 1.0 + state.correct_doors_taken - state.incorrect_doors_taken - 
          state.incorrect_exits_taken;
      // Truncate negative scores
      return (score >= 0) ? score : 0;
    }

    /// Calculate and store the score for the given state
    double UpdateScore(DoorsState& state){
      state.score = GetScore(state);
      return state.score;
    }

    /// Updates the records in the organism's traits
    void UpdateRecords(const DoorsState& state, org_t& org, 
        const EvalDoors_TraitNames& trait_names){
      org.SetTrait<size_t>(trait_names.door_rooms_trait, state.door_rooms_visited);
      org.SetTrait<size_t>(trait_names.exit_rooms_trait, state.exit_rooms_visited);
      org.SetTrait<size_t>(trait_names.correct_doors_trait, state.correct_doors_taken);
      org.SetTrait<size_t>(trait_names.incorrect_doors_trait, state.incorrect_doors_taken);
      org.SetTrait<size_t>(trait_names.correct_exits_trait, state.correct_exits_taken);
      org.SetTrait<size_t>(trait_names.incorrect_exits_trait, state.incorrect_exits_taken);
    }

    /// Calculate the score for the given state
    double GetDoorAccuracy(const DoorsState& state) const{
      if(state.door_rooms_visited <= 0) return 0;
      return static_cast<double>(state.correct_doors_taken) 
             / static_cast<double>(state.door_rooms_visited);
    }

    /// Extract the cue mask from the given string (one char (1 or 0) per cue)
    void ParseCuesMask(const std::string& s){ 
      random_cues_mask.Resize(s.size());
      for(size_t idx = 0; idx < s.size(); ++idx){
        if(s[idx] == '0') random_cues_mask[idx] = 0;
        else if(s[idx] == '1') random_cues_mask[idx] = 1;
        else emp_error("Error! ParseCuesMask only works on strings with 1s and 0s!");
      }
    }

    /// Fetch a random cue from the set
    DoorsState::data_t GetRandomCue(const DoorsState& state){
      // Offset so we don't return the exit cue
      return state.cue_vec[(rand.GetUInt() % (GetNumDoors() - 1)) + 1];
    }

    /// Initialize all properties of a DoorsState to prepare it for the task
    void InitializeState(DoorsState& state){
      state.initialized = true;
      state.score = 0;
      state.correct_doors_taken = 0;   
      state.incorrect_doors_taken = 0; 
      state.correct_exits_taken = 0;   
      state.incorrect_exits_taken = 0; 
      state.door_rooms_visited = 0; 
      state.exit_rooms_visited = 0; 
      // Randomize the cue vector according to configuration
      state.cue_vec.resize(GetNumDoors());
      size_t deterministic_cue_counter = 1;
      for(size_t idx = 0; idx < GetNumDoors(); ++idx){
        if(!random_cues_mask[idx]){ // If deterministic, use counter and increment
          state.cue_vec[idx] = deterministic_cue_counter++;
        }
        else{ // Randomize cue
          bool pass = false;
          while(!pass){
            pass = true;
            state.cue_vec[idx] = rand.GetUInt();
            // Ensure we didn't choose an existing cue
            for(size_t idx_2 = 0; idx_2 < idx; ++idx_2){
              if(state.cue_vec[idx] == state.cue_vec[idx_2]){
                pass = false;
                break;
              }
            }
          }
        }
      }
      // Set the initial cue
      state.current_cue = GetRandomCue(state);
    }
    
    /// Move the organism through its chosen door
    double Move(DoorsState& state, DoorsState::data_t door_idx){
      if(!state.initialized) InitializeState(state);
      // Increase room variable for correct room
      if(state.current_cue == state.cue_vec[exit_cue_idx]) state.exit_rooms_visited++;
      else state.door_rooms_visited++;
      if(door_idx == exit_cue_idx) return TakeExit(state);
      // Correct door -> Reward and move on!
      if(state.cue_vec[door_idx] == state.current_cue){
        state.correct_doors_taken++;
        state.prev_room_vec.push_back(state.current_cue);
        state.door_choice_vec.push_back(state.cue_vec[door_idx]);
        state.current_cue = GetRandomCue(state);
      }
      // Wrong door -> Penalize and move into "wrong" room
      else{
        state.incorrect_doors_taken++;
        state.prev_room_vec.push_back(state.current_cue);
        state.door_choice_vec.push_back(state.cue_vec[door_idx]);
        state.current_cue = state.cue_vec[exit_cue_idx];
      }
      return UpdateScore(state);
    }
    
    /// Move the organism through the "exit" door, going back one room  
    double TakeExit(DoorsState& state){
      if(!state.initialized) InitializeState(state);
      // Update bookkeeping
      state.prev_room_vec.push_back(state.current_cue);
      state.door_choice_vec.push_back(state.cue_vec[exit_cue_idx]);
      // Update score vars and current cue
      if(state.current_cue == state.cue_vec[exit_cue_idx]){
        state.correct_exits_taken++;
        state.current_cue = *(state.prev_room_vec.rbegin() + 1); // Return to previous room
      }
      else{
        state.incorrect_exits_taken++;
        state.current_cue = state.cue_vec[exit_cue_idx];
      }
      return UpdateScore(state);
    }


    /// Fetch the cue value of the organism's current room 
    //
    // Note: While it sounds like this should be a const method, it is possible this is the
    //  organism's first action, so we may need to initialize it
    DoorsState::data_t Sense(DoorsState& state) { 
      if(!state.initialized) InitializeState(state);
      return state.current_cue;
    }
  };

  /// \brief MABE module that evaluates Avida-esque organisms on how well they can associate cues to doors 
  class EvalDoors : public Module {
  public:
    using data_t = uint32_t;
    using org_t = VirtualCPUOrg;
    using inst_func_t = org_t::inst_func_t;

  protected:
    DoorsEvaluator evaluator;          /**< The evaluator that does all of the actual 
                                            computation and bookkeeping for the task*/
    int pop_id = 0;                    /**< ID of the population to evaluate and provide 
                                            instructions to */
    std::string randomize_cues_str;    /**< String version of a bitmask that determines 
                                            which cues are randomized */
    std::string inst_id_str;           ///< Semicolon-separated list of instruction door IDs  
    emp::vector<int> inst_id_vec;      ///< ID of the `sense` instruction
    int sense_inst_id = -1;
    EvalDoors_TraitNames trait_names;
    
    /// Parse the instruction ID string into an actual vector of ID numbers
    void ParseInstIDs(){
      inst_id_vec.clear();
      emp::vector<std::string> sliced_str_vec;
      emp::slice(inst_id_str, sliced_str_vec, ';');
      for(std::string& slice : sliced_str_vec){
        inst_id_vec.push_back(std::stoi(slice));
      }
    }

  public:
    EvalDoors(mabe::MABE & control,
                const std::string & name="EvalDoors",
                const std::string & desc="Evaluate organisms by how well they can associate symbols to doors.")
      : Module(control, name, desc)
      , evaluator(control.GetRandom())
    {
      SetEvaluateMod(true);
    }
    ~EvalDoors() { }

    /// Set up variables for configuration script
    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", "Population to evaluate.");
      LinkVar(trait_names.score_trait, "score_trait", 
           "Which trait stores task performance?");
      LinkVar(trait_names.accuracy_trait, "accuracy_trait", 
          "Which trait stores organism accuracy?");
      LinkVar(trait_names.state_trait, "state_trait", 
          "Which trait stores organisms' task state?");
      LinkVar(trait_names.door_rooms_trait, "door_rooms_trait", 
          "Which trait stores the number of \"door rooms\" visited?");
      LinkVar(trait_names.exit_rooms_trait, "exit_rooms_trait", 
          "Which trait stores the number of \"exit rooms\" visited?");
      LinkVar(trait_names.correct_doors_trait, "correct_doors_trait", 
          "Which trait stores the number of doors correctly taken?");
      LinkVar(trait_names.incorrect_doors_trait, "incorrect_doors_trait", 
          "Which trait stores the number of doors incorrectly taken?");
      LinkVar(trait_names.correct_exits_trait, "correct_exits_trait", 
          "Which trait stores the number of exits correctly taken?");
      LinkVar(trait_names.incorrect_exits_trait, "incorrect_exits_trait", 
          "Which trait stores the number of exits incorrectly taken?");
      LinkVar(randomize_cues_str, "randomize_cues_mask", "A string of 1s and 0s. Starting at "
          "cue 1, a 1 indicates that cue is random, while 0 is fixed.");
      LinkVar(inst_id_str, "inst_ids", "The IDs of the door instructions. Should be a string "
          "composed of numbers separated by semicolons (e.g., \"1;2;3\"");
      LinkVar(sense_inst_id, "sense_inst_id", "ID of the sense instruction"); 
    }
    
    /// Set up organism traits, load maps, and provide instructions to organisms
    void SetupModule() override {
      AddSharedTrait<double>(trait_names.score_trait, "EvalDoors score", 0.0);
      AddSharedTrait<double>(trait_names.accuracy_trait, "EvalDoors accuracy", 0.0);
      AddOwnedTrait<DoorsState>(trait_names.state_trait, "Organism's EvalDoors state", { }); 
      AddOwnedTrait<size_t>(trait_names.door_rooms_trait, "\"Door rooms\" visited", 0);
      AddOwnedTrait<size_t>(trait_names.exit_rooms_trait, "\"Exit rooms\" visited", 0);
      AddOwnedTrait<size_t>(trait_names.correct_doors_trait, "Correct doors taken", 0);
      AddOwnedTrait<size_t>(trait_names.incorrect_doors_trait, "Incorrect doors taken", 0);
      AddOwnedTrait<size_t>(trait_names.correct_exits_trait, "Correct exits taken", 0);
      AddOwnedTrait<size_t>(trait_names.incorrect_exits_trait, "Incorrect exits taken", 0);
      evaluator.ParseCuesMask(randomize_cues_str);
      SetupInstructions();
    }
    
    /// Package actions (e.g., sense, take door N) into instructions and provide them to the 
    /// organisms via ActionMap
    void SetupInstructions(){
      ParseInstIDs();
      ActionMap& action_map = control.GetActionMap(pop_id);
      // Add the correct number of door instructions
      for(size_t door_idx = 0; door_idx < inst_id_vec.size(); ++door_idx){
        inst_func_t func_move = [this, door_idx](org_t& hw, const org_t::inst_t& /*inst*/){
          DoorsState& state = hw.GetTrait<DoorsState>(trait_names.state_trait);
          double score = evaluator.Move(state, door_idx);
          hw.SetTrait<double>(trait_names.score_trait, score);
          hw.SetTrait<double>(trait_names.accuracy_trait, evaluator.GetDoorAccuracy(state));
          evaluator.UpdateRecords(state, hw, trait_names);
        };
        std::stringstream sstr;
        sstr << "doors-move-" << door_idx;
        Action& action = action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            sstr.str(), func_move);
        action.data.AddVar<int>("inst_id", inst_id_vec[door_idx]);
      }
      { // Sense 
        inst_func_t func_sense = [this](org_t& hw, const org_t::inst_t& inst){
          uint32_t val = evaluator.Sense(hw.GetTrait<DoorsState>(trait_names.state_trait));
          size_t reg_idx = inst.nop_vec.empty() ? 1 : inst.nop_vec[0];
          hw.regs[reg_idx] = val;
        };
        Action& action = action_map.AddFunc<void, org_t&, const org_t::inst_t&>(
            "doors-sense", func_sense);
        action.data.AddVar<int>("inst_id", sense_inst_id);
      }
    }
  };

  MABE_REGISTER_MODULE(EvalDoors, 
      "Evaluate organisms on their ability to associate symbols to doors.");
}

#endif
