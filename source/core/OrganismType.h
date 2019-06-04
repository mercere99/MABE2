/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  OrganismType.h
 *  @brief Details about how a specific type of organism should function.
 */

#ifndef MABE_ORGANISM_TYPE_H
#define MABE_ORGANISM_TYPE_H

#include "base/unordered_map.h"

namespace mabe {

  class OrganismTypeBase {
  private:
    std::string name;

  public:
    const std::string & GetName() { return name; }
  };

  template <typename ORG_T>
  class OrganismType : public OrganismTypeBase {
  };
}

#endif
