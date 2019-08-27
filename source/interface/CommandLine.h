/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
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
  public:
    CommandLine(mabe::MABE & control)
      : Module(control, "CommandLine", "Module to handle basic I/O on the command line.")
    {
      SetErrorHandleMod();
    }
    ~CommandLine() { }

    void SetupConfig(ConfigScope & config_scope) override {
      // For now, nothing here.
    }

    void SetupModule() override {
      // For now, nothing here.
    }

    void OnUpdate(size_t ud) override {
      std::cout << "Update: " << ud << std::endl;
    }

    void OnError(const std::string & msg) override {
      std::cerr << "ERROR: " << msg << std::endl;
    }

    void OnWarning(const std::string & msg) override {
      std::cerr << "WARNING: " << msg << std::endl;
    }

  };

}

#endif
