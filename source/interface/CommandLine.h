/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  CommandLine.h
 *  @brief Module to output errors and warnings to the command line.
 */

#ifndef MABE_COMMAND_LINE_H
#define MABE_COMMAND_LINE_H

#include "../core/MABE.h"
#include "../core/Module.h"

namespace mabe {

  class CommandLine : public Module {
  private:
    int pop_id=0;  // Which population should we print stats about?

  public:
    CommandLine(mabe::MABE & control,
                const std::string & name="CommandLine",
                const std::string & desc="Module to handle basic I/O on the command line.")
      : Module(control, name, desc)
    {
      SetErrorHandleMod();
    }
    ~CommandLine() { }

    void SetupConfig() override {
      LinkPop(pop_id, "target_pop", "Which population should we print stats about?");
    }

    void SetupModule() override {
      // For now, nothing here.
    }

    void OnUpdate(size_t ud) override {
      std::cout << "Update:" << ud;
      for (size_t pop_id = 0; pop_id < control.GetNumPopulations(); pop_id++) {
        const Population & pop = control.GetPopulation(pop_id);
        std::cout << "  " << pop.GetName() << ":" << pop.GetNumOrgs();
      }
      std::cout << std::endl;
    }

    void BeforeExit() override {
      auto & pop = control.GetPopulation(pop_id);
      std::cout << "Exiting.  Population " << pop.GetName()
                << " has " << pop.GetNumOrgs() << " organisms.";
      if (pop.GetNumOrgs()) {
        size_t pos = 0;
        while (pop[pos].IsEmpty()) pos++;
        std::cout << "  First org:\n" << pop[pos].ToString();
      }
      std::cout << std::endl;
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
