/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Module.h
 *  @brief Base class for all MABE modules.
 */

#ifndef MABE_MODULE_H
#define MABE_MODULE_H

#include <string>

#include "base/map.h"
#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/map_utils.h"
#include "tools/reference_vector.h"

#include "../config/Config.h"

#include "MABE.h"
#include "ModuleBase.h"
#include "Population.h"
#include "TraitInfo.h"

namespace mabe {

  class MABE;

  class Module : public ModuleBase {
  public:
    Module(MABE & in_control, const std::string & in_name, const std::string & in_desc="")
      : ModuleBase(in_control, in_name, in_desc) { ; }
    Module(const Module &) = default;
    Module(Module &&) = default;

  protected:
    
    // ---==  Configuration Management ==---

    /// Link a variable to a configuration entry - it sets the new default and
    /// automatically updates when configs are loaded.
    template <typename VAR_T, typename DEFAULT_T>
    ConfigEntry_Linked<VAR_T> & LinkVar(VAR_T & var,
                                        const std::string & name,
                                        const std::string & desc,
                                        DEFAULT_T default_val) {
      return control.GetCurScope().LinkVar<VAR_T>(name, var, desc, default_val);
    }

    /// Link a configuration entry to a pair of functions - it sets the new default and
    /// automatically calls the set function when configs are loaded.
    template <typename VAR_T, typename DEFAULT_T>
    ConfigEntry_Functions<VAR_T> & LinkFuns(std::function<VAR_T()> get_fun,
                                            std::function<void(const VAR_T &)> set_fun,
                                            const std::string & name,
                                            const std::string & desc,
                                            DEFAULT_T default_val) {
      return control.GetCurScope().LinkFuns<VAR_T>(name, get_fun, set_fun, desc, default_val);
    }

    /// Link a population to an ID tracker.
    ConfigEntry_Functions<std::string> & LinkPop(size_t & var,
                                                 const std::string & name,
                                                 const std::string & desc,
                                                 size_t default_pop=0) {
      std::function<std::string()> get_fun =
        [this,&var](){ return control.GetPopulation(var).GetName(); };
      std::function<void(std::string)> set_fun =
        [this,&var](const std::string & name){ var = control.GetPopID(name); };
      return control.GetCurScope().LinkFuns<std::string>(        
        name, get_fun, set_fun, desc,
        control.GetPopulation(default_pop).GetName());
    }

  };

}

#endif
