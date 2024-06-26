/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2024.
 *
 *  @file  EvalFunction.hpp
 *  @brief MABE Evaluation module rates organism's ability to perform a specified math function.
 * 
 *  This module specifies a function that agents are then evaluated based on how well they perform
 *  the function.
 */

#ifndef MABE_EVAL_FUNCTION_HPP
#define MABE_EVAL_FUNCTION_HPP

#include "emp/base/array.hpp"
#include "emp/base/notify.hpp"
#include "emp/data/Datum.hpp"
#include "emp/math/constants.hpp"

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

namespace mabe {

  class EvalFunction : public Module {
  private:
    static const constexpr size_t MAX_INPUTS = 5;

    emp::String input_traits = "input1,input2"; ///< Traits to put input value(s) for organism.
    emp::String output_trait = "output";        ///< Trait to find output values from organism.
    emp::String errors_trait = "errors";        ///< Trait for each test's deviation from target.
    emp::String fitness_trait = "fitness";      ///< Trait for combined fitness (#tests - error sum)

    // Track the DataMap ID for each trait or trait set.

    emp::array<size_t, MAX_INPUTS> input_ids;
    size_t output_id = emp::MAX_SIZE_T;
    size_t errors_id = emp::MAX_SIZE_T;
    size_t fitness_id = emp::MAX_SIZE_T;

    emp::String function = "input1 * 3 + 5*input2"; ///< Function to specify target output.

    /// Test values of each input in order, separated by a ';'
    //emp::String test_summary = "0:100; 100:-1:0"
    emp::String case_ids = 0:100;
    emp::String test_summary = "case_id; (case_id*7)%100";

    emp::vector<emp::String> input_names;   ///< Names of individual input traits.
    std::function<emp::Datum( const emp::DataMap & )> fit_fun;
    emp::vector<emp::vector<double>> test_values;
    emp::vector<double> target_results;
    size_t num_tests = emp::MAX_SIZE_T;

  public:
    EvalFunction(mabe::MABE & control,
                 emp::String name="EvalFunction",
                 emp::String desc="Evaluate organisms by having them solve a function.")
      : Module(control, name, desc)
    {
      SetEvaluateMod(true);
    }
    ~EvalFunction() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
                             [](EvalFunction & mod, Collection orgs) { return mod.Evaluate(orgs); },
                             "Evaluate organism's ability to solve a target function.");
    }

    void SetupConfig() override {
      LinkVar(input_traits, "input_traits", "Traits to put input value(s) for organism.\nFormat: comma-separated list");
      LinkVar(output_trait, "output_trait", "Trait to find output values from organism.");
      LinkVar(errors_trait, "errors_trait", "Trait for each test's deviation from target.");
      LinkVar(fitness_trait, "fitness_trait", "Trait for combined fitness (#tests - error sum)");
      LinkVar(function, "function", "Function to specify target output.");
      LinkVar(test_summary, "test_values", "Test values to use for evaluation.\nFormat: Range list for each variable; use ';' to separate variables");
      );
    }

    void SetupModule() override {
      input_names = emp::slice(input_traits, ',');
      if (intput_names.size() > MAX_INPUTS) {
        emp::notify::Error("EvalFunction does not allow more than ", MAX_INPUTS, " inputs. ",
                           input_names.size(), " inputs, requested.");
      }
      for (const emp::String & name : input_names) {
        AddOwnedTrait<double>(name, "Input value", 0.0);
      }
      AddRequiredTrait<double>(output_trait); // Output values
      AddOwnedTrait<emp::vector<double>>(errors_trait, "Error vector for tests.", emp::vector<double>();
      AddOwnedTrait<double>(fitness_trait, "Combined success rating", 0.0);

      // Prepare the test values to use.
      emp::remove_whitespace(test_summary);
      emp::vector<emp::String> test_sets = emp::slice(test_summary, ';');

      if (test_sets.size() != input_names.size()) {
        emp::notify::Error("EvalFunction requires one test set for each input.  Found ",
                           input_names.size(), " inputs, but ", tests_sets.size(), " test sets.");
      }

      // Put the test values in place, along with the expected results.
      test_values.resize(test_sets.size());               // Create one array for each input variable.
      for (size_t i = 0; i < test_sets.size(); ++i) {
        test_values[i] = emp::ToSequence(test_sets[i]);   // Set all values for one input variable.
        if (num_tests == emp::MAX_SIZE_T) num_tests = test_values[i].size();
        else if (test_values[i].size() != num_tests) {
          emp::notify::Error("EvalFunction requires all inputs to have the same count of values.  First input (0) has ",
                            test_values[0].size(), " test values, but ", i, " has ", test_values[i].size(), ".");
        }
      }

      // Build a DataMap to determine expected results for each test case.
      emp::DataMap test_map;
      for (size_t i = 0; i < input_names.size(); ++i) {
        test_map.AddVar<double>(input_names[0]);
      }

    }


    double Evaluate(const Collection & orgs) {
      // If we haven't calculated the IDs, do so now.
      if (output_trait == emp::MAX_SIZE_T) {
        input_ids.resize(input_names.size());
        for (size_t i = 0; i < input_names.size(); ++i) {
          input_ids[i] = orgs.GetDataLayout().GetID(input_names[i]);
        }
        output_id = orgs.GetDataLayout().GetID(output_trait);
        errors_id = orgs.GetDataLayout().GetID(errors_trait);
        fitness_id = orgs.GetDataLayout().GetID(fitness_trait);
      }

      // Loop through the living organisms in the target collection to evaluate each.
      mabe::Collection alive_collect( orgs.GetAlive() );

      control.Verbose(" - ", alive_collect.GetSize(), " organisms found.");

      size_t org_count = 0;
      double max_fitness = 0.0;
      for (Organism & org : alive_collect) {
        control.Verbose("...eval org #", org_count++);

        double & ouput = org.GetTrait<double>(output_id);
        double & errors = org.GetTrait<emp::vector<double>>(errors_id);
        double & fitness = org.GetTrait<double>(fitness_trait);

        /// Loop through test cases to evaluate each.
        for (size_t test_id = 0; test_id < num_tests; ++test_id) {
          // Setup inputs for the current test.
          for (size_t input_pos = 0; input_pos < input_ids.size(); ++input_pos) {
            org.SetTrait(input_ids[input_pos], test_values[input_pos][test_id]);
          }

          // Run the organism.
          org.GenerateOutput();

          // Evaluate the results.

        }

        if (fitness > max_fitness) max_fitness = fitness;
      }

      return max_fitness;
    }

    // If a population is provided to Evaluate, first convert it to a Collection.
    double Evaluate(Population & pop) { return Evaluate( Collection(pop) ); }

    // If a string is provided to Evaluate, convert it to a Collection.
    double Evaluate(const emp::String & in) { return Evaluate( control.ToCollection(in) ); }
  };

  MABE_REGISTER_MODULE(EvalFunction, "Evaluate organisms on their ability to produce a target function.");
}

#endif
