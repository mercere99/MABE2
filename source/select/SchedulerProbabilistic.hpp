/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  SchedulerProbabilistic.h
 *  @brief Rations out updates to organisms based on a specified attribute, using a method akin to roulette selection. 
 **/

#ifndef MABE_SCHEDULER_PROB_H
#define MABE_SCHEDULER_PROB_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"
#include "emp/datastructs/UnorderedIndexMap.hpp"

namespace mabe {

  /// Rations out updates to organisms based on a specified attribute, using a method akin to roulette selection  
  class SchedulerProbabilistic : public Module {
  private:
    std::string trait = "merit";  ///< Which trait should we select on?
    std::string reset_self_trait = "reset_self";  ///< What should we call the trait used to track resetting?
    double avg_updates = 0; ///< How many updates should organisms receive on average?
    int pop_id = 0;     ///< Which population are we selecting from?
    emp::UnorderedIndexMap weight_map; ///< Data structure storing all organism fitnesses
    double base_value = 1; ///< Fitness value that all organisms start with 
    double merit_scale_factor = 1; ///< Fitness = base_value + (merit * this value)
  public:
    SchedulerProbabilistic(mabe::MABE & control,
                     const std::string & name="SchedulerProbabilistic",
                     const std::string & desc="Rations out updates to organisms based on a specified attribute, using a method akin to roulette wheel selection",
                     const std::string & in_trait="merit",
                     size_t in_avg_updates = 30)
      : Module(control, name, desc)
      , trait(in_trait), avg_updates(in_avg_updates)
      , weight_map()
    {
    }
    ~SchedulerProbabilistic() { }

    /// Set up variables for configuration file
    void SetupConfig() override {
      LinkPop(pop_id, "pop", "Which population should we select parents from?");
      LinkVar(avg_updates, "avg_updates", "How many updates should organism receive on average?");
      LinkVar(trait, "trait", "Which trait provides the fitness value to use?");
      LinkVar(reset_self_trait, "reset_self_trait", 
          "Name of the trait tracking if an organism should reset itself");
      LinkVar(base_value, "base_value", "What value should the scheduler use for organisms"
          " that have performed no tasks?");
      LinkVar(merit_scale_factor, "merit_scale_factor", "How should the scheduler scale merit?");
    }

    /// Register traits
    void SetupModule() override {
      AddRequiredTrait<double>(trait); ///< The fitness trait must be set by another module.
      AddOwnedTrait<bool>(reset_self_trait, "Does org need reset?", false); ///< Allow organisms to reset themselves 
    }

    /// Set up member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction(
        "SCHEDULE",
        [](SchedulerProbabilistic & mod) {
          return mod.Schedule();
        },
        "Perform one round of scheduling");
    }

    /// Ration out updates to members of the population
    double Schedule() {
      // Grab the variables we'll use repeatedly 
      emp::Random & random = control.GetRandom();
      Population & pop = control.GetPopulation(pop_id);
      const size_t N = pop.GetSize();
      // Make sure the population isn't empty
      if (pop.GetNumOrgs() == 0) {
        emp::notify::Error("Trying to schedule an empty population.");
        return 0;
      }

      if(weight_map.GetSize() == 0) weight_map.Resize(N, base_value);
      size_t selected_idx;
      // Dole out updates
      for(size_t i = 0; i < N * avg_updates; ++i){
        const double total_weight = weight_map.GetWeight();
        if(total_weight > 0.0){ // TODO: cap to max weight
          selected_idx = weight_map.Index(random.GetDouble() * total_weight);
        }
        else selected_idx = random.GetUInt(pop.GetSize()); // No weights -> pick randomly 
        pop[selected_idx].ProcessStep();
      }
      return weight_map.GetWeight();
    }

    /// When an organism is placed in a population, add its weight to the weight map
    void OnPlacement(OrgPosition placement_pos){
      Population & pop = placement_pos.Pop();
      const size_t N = pop.GetSize();
      if(weight_map.GetSize() < N){
        weight_map.Resize(N, 1);
      }
      size_t org_idx = placement_pos.Pos();
      weight_map.Adjust(org_idx, 
          base_value + merit_scale_factor * placement_pos.Pop()[org_idx].GetTrait<double>(trait));
      placement_pos.Pop()[org_idx].SetTrait<bool>(reset_self_trait, false);
    }
  };

  MABE_REGISTER_MODULE(SchedulerProbabilistic, "Rations out updates to organisms based on a specified attribute, using a method akin to roulette wheel selection.");
}

#endif
