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

#include <string>

#include "emp/base/vector.hpp"
#include "emp/base/notify.hpp"
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

    bool exit_now = false;                    ///< Has something gone wrong and we should abort?

    // --- Private helper functions ---
    template <typename... Ts>
    bool Require(bool test, Ts &&... args) {
      if (!test) {
        std::cout << "ERROR: ";
        ((std::cout << std::forward<Ts>(args)), ...);
      }
      return test;
    }

    bool Process_Factor(std::string line) {
      if (!Require(line.size(), "Factors must have a factor name.")) return false;
      std::string name = emp::string_pop_word(line);
      factors.push_back(name);
      if (!Require(line.size(), "Factor '", name, "' must have at least one value.")) return false;

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
          config_options.push_back(line);
        } else if (keyword == "factor") {        // A range of variables to try in all combinations
          bool result = ProcessFactor(line);
          if (!result) return;
        } else if (keyword == "log") {      // A file to log output of runs
          log_file = emp::string_pop_word(line);
          Require(line.size() == 0, "Only filename should be specified in 'log'; using '", log_file, "'.");
        } else if (keyword == "mabe") {          // Set the mabe executable location
          exe_name = emp::string_pop_word(line);
          Require(line.size() == 0, "Only one executable should be specified in 'mabe'; using '", exe_name, "'.");
        } else if (keyword == "replicate") {     // Provide num replicates for each combo
          replicates = emp::from_string<int>(emp::string_pop_word(line));
          Require(line.size() == 0, "Only one value should be specified in 'replicate'; using '", replicates, "'.");
        } else if (keyword == "set") {           // Set a local variable value

        } else {
          std::cerr << "ERROR: Unknown keyword '" << keyword << "'.  Aborting." << std::endl;
          return;
        }
    }      
    }
  };

}

#endif
