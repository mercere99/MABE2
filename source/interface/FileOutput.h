/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  FileOutput.h
 *  @brief Module to output collected data into a specified file.
 */

#ifndef MABE_FILE_OUTPUT_H
#define MABE_FILE_OUTPUT_H

#include "../core/MABE.h"
#include "../core/Module.h"

namespace mabe {

  class FileOutput : public Module {
  private:
    std::string filename;

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
      LinkPop(filename, "filename", "Name of file to print into.");
    }

    void SetupModule() override {
      // For now, nothing here.
    }

    void OnUpdate(size_t ud) override {
      // @CAO: Print data to file.
    }

    void BeforeExit() override {
      // @CAO: Clost file.
    }

  };

  MABE_REGISTER_MODULE(FileOutput, "Output collected data into a specified file.");
}

#endif
