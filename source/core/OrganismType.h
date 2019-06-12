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
    const std::string & GetName() const { return name; }
  };

  template <typename ORG_T>
  class OrganismType : public OrganismTypeBase {
  };

   
  struct OrganismTypeManager {
    using type_map_t = emp::unordered_map<std::string, emp::Ptr<OrganismTypeBase>>;

    static type_map_t & GetMap() {
      static type_map_t type_map;
      return type_map;
    }    

    template <typename T>
    static OrganismType<T> & Get(const std::String & name) {
      type_map_t & type_map = GetMap();
      auto it = type_map.find(name);
      if (it == type_map.end()) {
        auto new_type = emp::NewPtr<OrganismType<T>>(name);
        type_map[name] = new_type;
        return *new_type;
      }
      return *(it->second);
    }
  };
}

#endif
