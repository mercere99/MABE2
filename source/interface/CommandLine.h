/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ErrorHandleCL.h
 *  @brief Module to output errors and warnings to the command line.
 */

#ifndef MABE_ERROR_HANDLE_CL_H
#define MABE_ERROR_HANDLE_CL_H

#include "../core/MABE.h"
#include "../core/Module.h"

namespace mabe {

  class ErrorHandleCL : public Module {
  public:
    ErrorHandleCL(mabe::MABE & control)
      : Module(control, "ErrorHandleCL", "Module to output errors and warnings to the command line.")
    {
      SetErrorHandleMod();
    }
    ~ErrorHandleCL() { }

    void SetupConfig(ConfigScope & config_scope) override {
      // For now, nothing here.
    }

    void SetupModule() override {
      // For now, nothing here.
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
