/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022.
 *
 *  @file  Batch.hpp
 *  @brief Manager for batches of MABE runs.
 */

#ifndef MABE_BATCH_HPP
#define MABE_BATCH_HPP

#include <cstdlib>
#include <string>
#include <unordered_map>

#include "emp/base/vector.hpp"
#include "emp/base/notify.hpp"
#include "emp/io/File.hpp"
#include "emp/tools/string_utils.hpp"

namespace mabe {

  class Batch {
  private:
    struct FactorInfo {
      std::string name;
      emp::vector<std::string> options;
      FactorInfo(const std::string & _name) : name(_name) { }
    };

    emp::File batch_file;
    std::string exe_name;

    emp::vector<std::string> config_options;  ///< Options to use on the command line.
    emp::vector<FactorInfo> factors;          ///< Set of factors to combinatorially vary.
    std::string log_file;                     ///< Where should run details be saved?
    int replicates = 1;                       ///< How many replicates of each factor combination?

    std::unordered_map<std::string, std::string> var_set; ///< Variable to use in script.

    bool exit_now = false;                    ///< Has something gone wrong and we should abort?

    // --- Private helper functions ---
    template <typename... Ts>
    bool Require(bool test, Ts &&... args) {
      if (exit_now) return false; // Already had a failure; don't report multiple.

      if (!test) {
        emp::notify::Error(std::forward<Ts>(args)...);
        exit_now = true;
      }
      return test;
    }

    bool Process_Factor(std::string line) {
      Require(line.size(), "Factors must have a factor name.");
      std::string name = emp::string_pop_word(line);
      factors.push_back(name);
      Require(line.size(), "Factor '", name, "' must have at least one value.");

      while (line.size()) {
        std::string option = emp::string_pop_word(line);
        factors.back().options.push_back(option);
      }

      return true;
    }

  public:
    Batch(const std::string & filename, const std::string & _exe_name) 
    : batch_file(filename), exe_name(_exe_name) {
      batch_file.RemoveComments('#');
      batch_file.CompressWhitespace();
    }

    void Process() {
      // Loop through batch file, processing line-by-line.
      for (std::string line : batch_file) {
        std::string keyword = emp::string_pop_word(line);
        if (keyword == "config") {               // Set a config options on command line
          Require(line.size(), "'config' must specify option to include.");
          config_options.push_back(line);
        } else if (keyword == "factor") {        // A range of variables to try in all combinations
          bool result = Process_Factor(line);
          if (!result) return;
        } else if (keyword == "log") {      // A file to log output of runs
          Require(line.size(), "'log' must specify filename.");
          log_file = emp::string_pop_word(line);
          Require(!line.size(), "Only filename should be specified in 'log'; text follows '", log_file, "'.");
        } else if (keyword == "mabe") {          // Set the mabe executable location
          Require(line.size(), "'mabe' must specify executable.");
          exe_name = emp::string_pop_word(line);
          Require(!line.size(), "Only one executable should be specified in 'mabe'; text follows '", exe_name, "'.");
        } else if (keyword == "replicate") {     // Provide num replicates for each combo
          Require(line.size(), "'replicate' must specify number of replicates.");
          replicates = emp::from_string<int>(emp::string_pop_word(line));
          Require(!line.size(), "Only one value should be specified in 'replicate'; text follows '", replicates, "'.");
        } else if (keyword == "set") {           // Set a local variable value
          Require(line.size(), "'set' must specify variable name and value to set to.");
          std::string var = emp::string_pop_word(line);
          Require(var != "seed", "The variable 'seed' is reserved for the random number seed used.");
          var_set[var] = line;          
        } else {
          std::cerr << "ERROR: Unknown keyword '" << keyword << "'.  Aborting." << std::endl;
          return;
        }

        if (exit_now) return;
      }
    }

    void Run() {
      emp::notify::Message("Processing BATCH runs.");
      emp::notify::Message("Using ", factors.size(), " combined factors");
      for (const auto & factor : factors) {
        emp::notify::Message("  ", factor.name, " with ", factor.options.size(), " options.");
      }

      int seed = 1;  // Seeds start at 1 and work their way up.

      // Loop through combinations of factors.
      emp::vector<size_t> ids(factors.size());
      for (auto & x : ids) x = 0;

      while (true) {
        // Set variables using the current factors.
        for (size_t i = 0; i < ids.size(); ++i) {
          var_set[factors[i].name] = factors[i].options[ids[i]];
        }

        // Generate the base run string.
        std::stringstream ss;
        ss << exe_name;
        for (const std::string & option : config_options) {
          ss << " " << option;
        }
        ss << " -s random_seed={$seed}";

        // Do all replicates in this treatment.
        for (int i = 0; i < replicates; ++i) {
          // Prep the random seed.
          var_set["seed"] = seed++;

          // Substitute in variables.
          std::string exe_string(ss.str());        
          exe_string = emp::replace_vars(exe_string, var_set);

          // And run the executable
          emp::notify::Message("BATCH COMMAND: ", exe_string);
          std::system(exe_string.c_str());
        }

        // Move on to the next factors.
        size_t inc_pos = 0;
        while(inc_pos < factors.size() && ++ids[inc_pos] == factors[inc_pos].options.size()) {
          ids[inc_pos] = 0; // Reset the current factor.
          ++inc_pos;        // And move on to the next.
        }
        if (inc_pos == factors.size()) break; // We've gone through all factors!
      }
    }
  };

}

#endif
