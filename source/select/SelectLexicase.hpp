/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  SelectLexicase.hpp
 *  @brief MABE module to enable Lexicase selection
 */

#ifndef MABE_SELECT_LEXICASE_H
#define MABE_SELECT_LEXICASE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"
#include "../core/TraitSet.hpp"

#include "emp/datastructs/valsort_map.hpp"

namespace mabe {

  /// Add Lexicase selection with the current population.
  class SelectLexicase : public Module {
  private:
    std::string trait_inputs;  ///< Which set of trait values should we select on?
    TraitSet trait_set;        ///< Processed version of trait_inputs.
    double epsilon = 0.0;      ///< Range from max value to be preserved? (fraction of max)
    int select_pop_id = 0;     ///< Which population are we selecting from?
    int birth_pop_id = 1;      ///< Which population should births go into?
    size_t num_births = 1;     ///< How many offspring organisms should we produce?

  public:
    SelectLexicase(mabe::MABE & control,
               const std::string & name="SelectLexicase",
               const std::string & desc="Module to choose the top fitness organisms for replication.")
      : Module(control, name, desc)
    {
      SetSelectMod(true);    ///< Mark this module as a selection module.
    }
    ~SelectLexicase() { }

    void SetupConfig() override {
      LinkPop(select_pop_id, "select_pop", "Which population should we select parents from?");
      LinkPop(birth_pop_id, "birth_pop", "Which population should births go into?");
      LinkVar(trait_inputs, "fitness_traits", "Which traits provide the fitness values to use?");
      LinkVar(epsilon, "epsilon", "Range from max value to be preserved? (fraction of max)");
      LinkVar(num_births, "num_births", "Number of offspring organisms to produce")
    }

    void SetupModule() override {
      trait_set.SetTraits(trait_inputs);     ///< Parse set of trait inputs passed in.
      AddRequiredTraits<double>(trait_set);  ///< The fitness trait must be set by another module.
    }

    void OnUpdate(size_t update) override {
      // Construct a map of all IDs to their associated fitness values.
      emp::valsort_map<OrgPosition, double> id_fit_map;
      Collection select_col = control.GetAlivePopulation(select_pop_id);
      for (auto it = select_col.begin(); it != select_col.end(); it++) {
        id_fit_map.Set(it.AsPosition(), it->GetVar<double>(trait));
        //std::cout << "Measuring fit " << it->GetVar<double>(trait) << std::endl;
      }

      // Loop through the IDs in fitness order (from highest), replicating each
      size_t num_reps = 0;
      Population & birth_pop = control.GetPopulation(birth_pop_id);
      for (auto it = id_fit_map.crvbegin(); it != id_fit_map.crvend() && num_reps++ < top_count; it++) {
        //std::cout << "Replicating fit " << it->first->GetVar<double>(trait) << std::endl;
        control.Replicate(it->first, birth_pop, copy_count);
      }
    }
  };

  MABE_REGISTER_MODULE(SelectLexicase, "Choose the top fitness organisms for replication.");
}

#endif
