/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  SchedulerProbabilistic.h
 *  @brief Rations out updates to organisms based on a specified attribute, using a method akin to roulette wheel selection. 
 **/

#ifndef MABE_SCHEDULER_PROB_H
#define MABE_SCHEDULER_PROB_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"
#include "emp/datastructs/IndexMap.hpp"
namespace mabe {

  /// Rations out updates to organisms based on a specified attribute, used a method akin to roulette wheel selection  
  class SchedulerProbabilistic : public Module {
  private:
    std::string trait;       ///< Which trait should we select on?
    double avg_updates;      ///< How many updates should organisms receive on average?
    int pop_id = 0;   ///< Which population are we selecting from?
    emp::IndexMap weight_map;
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

    void SetupConfig() override {
      LinkPop(pop_id, "pop", "Which population should we select parents from?");
      LinkVar(avg_updates, "avg_updates", "How many updates should organism receive on average?");
      LinkVar(trait, "trait", "Which trait provides the fitness value to use?");
    }

    void SetupModule() override {
      AddRequiredTrait<double>(trait); ///< The fitness trait must be set by another module.
    }

    // TODO: Currently, the weight map is dropped and recreated every update. Switch to only updating 
    //          an organism's weight when it changes
    // TODO: Check to see if organism can get updates in the update they are born, and see if above todo 
    //          will do the right thing
    /// Ration out updates to members of the population
    void OnUpdate(size_t update) override {
      // Grab the variables we'll use over and over
      emp::Random & random = control.GetRandom();
      Population & pop = control.GetPopulation(pop_id);
      const size_t N = pop.GetSize();
      // Make sure the population isn't empty
      if (pop.GetNumOrgs() == 0) {
        control.AddError("Trying to schedule an empty population.");
        return;
      }
      size_t selected_idx;
      double total_weight = weight_map.GetWeight();
      // Dole out updates
      for(size_t i = 0; i < N * avg_updates; ++i){
        if(total_weight > 0)
          selected_idx = weight_map.Index(random.GetDouble() * total_weight);
        else
          selected_idx = random.GetUInt(pop.GetSize()); 
        // If ProcessStep returns true, org needs to replicate (will be reworked soon!)
        if(pop[selected_idx].ProcessStep()){
          control.Replicate(OrgPosition(pop, selected_idx), pop, 1, true); 
          weight_map.Adjust(selected_idx, pop[selected_idx].GetVar<double>(trait));
        }
      }
      std::cout << "Total weight: " << total_weight << std::endl;
    }
 
    void OnPlacement(OrgPosition placement_pos){
      if(weight_map.GetSize() == 0){
        Population & pop = control.GetPopulation(pop_id);
        const size_t N = pop.GetSize();
        weight_map.ResizeClear(N);
      }
      size_t org_idx = placement_pos.Pos();
      weight_map.Adjust(org_idx, placement_pos.Pop()[org_idx].GetVar<double>(trait));
    }
  };

  MABE_REGISTER_MODULE(SchedulerProbabilistic, "Rations out updates to organisms based on a specified attribute, using a method akin to roulette wheel selection.");
}

#endif
