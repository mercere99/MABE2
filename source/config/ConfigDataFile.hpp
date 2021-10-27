/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  ConfigDataFile.hpp
 *  @brief Manages a DataFile object for config.
 *  @note Status: BETA
 */

#ifndef MABE_CONFIG_DATA_FILE_H
#define MABE_CONFIG_DATA_FILE_H

#include <functional>
#include <string>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"

#include "ConfigType.hpp"

namespace mabe {

  /// A ConfigDataFile maintains an output file that has specified columns and can be generate
  /// dynamically.
  class ConfigDataFile : public ConfigType {
  private:
    using fun_t = std::function<std::string()>;
    struct ColumnInfo {
      std::string header;
      fun_t fun;
    };

    std::string name="";             ///< Unique name for this object.
    emp::StreamManager & files;      ///< Global file manager.

    std::string filename;            ///< Name of output file.
    emp::vector<ColumnInfo> cols;    ///< Data about columns maintainted.

  public:
    ConfigDataFile() = delete;
    ConfigDataFile(const std::string & in_name, emp::StreamManager & _files)
      : name(in_name), files(_files) { }
    ~ConfigDataFile() { }

    std::string GetName() const { return name; }

    // Setup member functions associated with population.
    static void InitType(Config & /*config*/, ConfigTypeInfo & info) {
      auto fun_num_cols = [](ConfigDataFile & target) { return target.cols.size(); };
      info.AddMemberFunction("NUM_COLS", fun_num_cols, "Return the number of columns in this file.");
      info.AddMemberFunction("WRITE", [](ConfigDataFile & target) { return target.Write(); },
                             "Add on the next line of data.");
    }

    void SetupConfig() override {
      LinkVar(filename, "filename", "Name to use for this file.");
    }

    size_t AddColumn(const std::string & header, fun_t fun) {
      size_t col_id = cols.size();
      cols.push_back(ColumnInfo{header,fun});
      return col_id;
    }

    size_t Write() {
      const bool file_exists = files.Has(filename);           // Is file is already setup?
      std::ostream & file = files.GetOutputStream(filename);  // File to write to.

      // If we need headers, set them up!
      if (!file_exists) {
        for (size_t i = 0; i < cols.size(); ++i) {
          if (i) file << ", ";
          file << cols[i].header;
        }
        file << '\n';
      }

      // Now print out each entry.
      for (size_t i = 0; i < cols.size(); ++i) {
        if (i) file << ", ";
        file << cols[i].fun();
      }
      file << std::endl;

      return 1;
    }

    static std::string EMPGetTypeName() { return "mabe::ConfigDataFile"; }
  };
}

#endif
