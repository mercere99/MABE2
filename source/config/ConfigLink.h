/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  ConfigLink.h
 *  @brief Links C++ variables to configuration options.
 *  @note Status: ALPHA
 */

#ifndef MABE_CONFIG_LINK_H
#define MABE_CONFIG_LINK_H

#include "base/assert.h"
#include "meta/TypeID.h"
#include "tools/string_utils.h"

namespace mabe {

  class ConfigLink_Base {
  protected:
    std::string name;
    std::string desc;

  public:
    ConfigLink_Base(const std::string & _name, const std::string & _desc="")
      : name(_name), desc(_desc) { }
    virtual ~ConfigLink_Base() { }

    const std::string & GetName() const { return name; }
    const std::string & GetDesc() const { return desc; }

    virtual void SetValue(double val) = 0;
    virtual void SetString(const std::string & val) = 0;
  };

  template <typename T>
  class ConfigLink : public ConfigLink_Base {
  protected:
    T & linked_var;   ///< A reference to the variable linked to this configuration option.
  public:
    ConfigLink(T & var, const std::string & name, const std::string & desc="")
      : ConfigLink_Base(name, desc), linked_var(var) { }
    ~ConfigLink() { }

    virtual void SetValue(double val) { linked_var = val; }
    virtual void SetString(const std::string & val) {linked_var = val; }
  };

}
#endif
