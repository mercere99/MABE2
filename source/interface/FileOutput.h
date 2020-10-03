/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  FileOutput.h
 *  @brief Module to output collected data into a specified file.
 * 
 *  Format for files is pretty versitile.  It is a comma separated list of the columns.
 * 
 *  Example:
 *   update, main_pop.ave.generation, main_pop.ave.score, main_pop.max.score
 */

#ifndef MABE_FILE_OUTPUT_H
#define MABE_FILE_OUTPUT_H

#include "tools/string_utils.h"

#include "../core/MABE.h"
#include "../core/Module.h"

namespace mabe {

  class FileOutput : public Module {
  private:
    std::string filename;
    std::string format;
    Collection target_collect;

    // Calculated values from the inputs.
    using trait_fun_t = std::function<std::string(const Collection &)>;
    emp::vector<std::string> cols;  // Names of the columns to use.
    emp::vector<trait_fun_t> funs;  // Functions to call each update.

  public:
    FileOutput(mabe::MABE & control,
               const std::string & name="FileOutput",
               const std::string & desc="Module to output collected data into a specified file.")
      : Module(control, name, desc)
    {
      SetInterfaceMod();
    }
    ~FileOutput() { }

    void SetupConfig() override {
      LinkVar(filename, "filename", "Name of file to print into.");
      LinkVar(format, "format", "Column format to use in the file.");
      LinkCollection(target_collect, "target", "Which population(s) should we print from?");
    }

    void SetupModule() override {
      emp::slice(format, cols, ',');
      funs.resize(cols.size());
      for (size_t i = 0; i < cols.size(); i++) {
        funs[i] = control.ParseTraitFunction(cols[i]);
      }
    }

    void OnUpdate(size_t ud) override {
      // @CAO: Print data to file.
    }

    void BeforeExit() override {
      // @CAO: Close file.
    }

  };

  MABE_REGISTER_MODULE(FileOutput, "Output collected data into a specified file.");
}

#endif
