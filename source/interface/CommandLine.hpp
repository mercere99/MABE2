/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  CommandLine.hpp
 *  @brief Module to output errors and warnings to the command line.
 */

#ifndef MABE_COMMAND_LINE_H
#define MABE_COMMAND_LINE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  class CommandLine : public Module {
  private:
    std::string format;
    Collection target_collect;

    // Calculated values from the inputs.
    using trait_fun_t = std::function<std::string(const Collection &)>;
    emp::vector<std::string> cols;  ///< Names of the columns to use.
    emp::vector<trait_fun_t> funs;  ///< Functions to call each update.
    bool init = false;

    void Initialize() {
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

      init = true;
    }

  public:
    CommandLine(mabe::MABE & control,
                const std::string & name="CommandLine",
                const std::string & desc="Module to handle basic I/O on the command line.")
      : Module(control, name, desc)
      , format("fitness:max,fitness:mean")
      , target_collect(control.GetPopulation(0))
    {
      SetInterfaceMod();
      SetErrorHandleMod();
    }
    ~CommandLine() { }

    void SetupConfig() override {
      LinkVar(format, "format", "Column format to use in the file.");
      LinkCollection(target_collect, "target", "Which population(s) should we print from?");
    }

    void SetupModule() override {
    }

    void BeforeUpdate(size_t ud) override {
      std::cout << "Update:" << ud;

      if (ud == 0) {               // At the very beginning, no stats available.
        std::cout << std::endl;
        return;
      }

      if (!init) Initialize();

      for (size_t pop_id = 0; pop_id < control.GetNumPopulations(); pop_id++) {
        const Population & pop = control.GetPopulation(pop_id);
        std::cout << "  " << pop.GetName() << ":" << pop.GetNumOrgs();
      }

      mabe::Collection cur_collect = target_collect.GetAlive();
      for (size_t i = 0; i < funs.size(); ++i) {
        std::cout << ", " << cols[i] << "=" << funs[i](cur_collect);
      }

      std::cout << std::endl;
    }

    void BeforeExit() override {
      std::cout << "==> Exiting." << std::endl;
    }

    void OnError(const std::string & msg) override {
      std::cerr << "ERROR: " << msg << std::endl;
    }

    void OnWarning(const std::string & msg) override {
      std::cerr << "WARNING: " << msg << std::endl;
    }

  };

  MABE_REGISTER_MODULE(CommandLine, "Handle basic I/O on the command line.");
}

#endif
