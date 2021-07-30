/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2021.
 *
 *  @file  FileOutput.hpp
 *  @brief Module to output collected data into a specified file.
 * 
 *  Format for files is pretty versitile.  It is a comma separated list of the columns.
 * 
 *  Example:
 *   update, main_pop.ave.generation, main_pop.ave.score, main_pop.max.score
 */

#ifndef MABE_FILE_OUTPUT_H
#define MABE_FILE_OUTPUT_H

#include <fstream>

#include "emp/tools/string_utils.hpp"

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  class FileOutput : public Module {
  private:
    std::string filename;
    std::string format;
    Collection target_collect;
    int start_ud=0;    ///< When should outputs start being printed?
    int step_ud=1;     ///< How often should outputs be printed?
    int stop_ud=-1;    ///< When should outputs stop being printed?
    bool init = false; ///< Has the file been initialized?

    // Calculated values from the inputs.
    using trait_fun_t = std::function<std::string(const Collection &)>;
    emp::vector<std::string> cols;  ///< Names of the columns to use.
    emp::vector<trait_fun_t> funs;  ///< Functions to call each update.
    std::ofstream file;

    // Setup the columns to be printed right before the first time we print
    // (to make sure all of the values we are using have known types.)
    void InitializeFile() {
      // Open the file that we will be writing to.
      file.open(filename);

      // Identify the contents of each column.
      emp::remove_whitespace(format);
      emp::slice(format, cols, ',');

      // Setup a function to collect data associated with each column.
      funs.resize(cols.size());
      for (size_t i = 0; i < cols.size(); i++) {
        std::string trait_filter = cols[i];
        std::string trait_name = emp::string_pop(trait_filter,':');
        funs[i] = control.BuildTraitFunction(trait_name, trait_filter);
      }

      // Print the headers into the file.
      file << "#update";
      for (size_t i = 0; i < cols.size(); i++) {
        file << ", " << cols[i];
      }
      file << '\n';

      init = true;
    }

    void DoOutput(size_t ud) {
      if (!init) InitializeFile();

      // Check if we should print this update.
      if (((int) ud < start_ud) ||
          (stop_ud != -1 && (int) ud > stop_ud) ||
          ((ud - start_ud)%step_ud != 0) ) return;

      // If so, print!
      file << ud;
      mabe::Collection cur_collect = target_collect.GetAlive();
      for (auto & fun : funs) {
        file << ", " << fun(cur_collect);
      }
      file << std::endl;
    }

  public:
    FileOutput(mabe::MABE & control,
               const std::string & name="FileOutput",
               const std::string & desc="Module to output collected data into a specified file.")
      : Module(control, name, desc), filename("output.csv"), format("fitness:max,fitness:mean"),
        target_collect(control.GetPopulation(0))
    {
      SetInterfaceMod();
    }
    ~FileOutput() { }

    void SetupConfig() override {
      LinkVar(filename, "filename", "Name of file for output data.");
      LinkVar(format, "format", "Column format to use in the file.");
      LinkCollection(target_collect, "target", "Which population(s) should we print from?");
      LinkRange(start_ud, step_ud, stop_ud, "output_updates", "Which updates should we output data?");
    }

    void SetupModule() override {
      // Nothing for now.
    }

    void BeforeUpdate(size_t ud) override {
      control.Verbose("UD ", ud, ": Running FileOutput::BeforeUpdate()");
      DoOutput(ud);
    }

    void BeforeExit() override {
      // Do a final printing at the end and close the file.
      DoOutput(control.GetUpdate());
      file.close();
    }

  };

  MABE_REGISTER_MODULE(FileOutput, "Output collected data into a specified file.");
}

#endif
