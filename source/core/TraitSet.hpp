/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  TraitSet.hpp
 *  @brief A collection of traits with the same type (or collections of that type).
 *
 *  A TraitSet is used to keep track of a collection of related traits in a module.
 */

#ifndef MABE_TRAIT_SET_H
#define MABE_TRAIT_SET_H

#include <string>

#include "emp/base/vector.hpp"
#include "emp/data/DataMap.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/tools/string_utils.hpp"

namespace mabe {

  template <typename T>
  class TriatSet {
  private:
    emp::vector<std::string> base_names;
    emp::vector<std::string> vector_names;
    emp::vector<size_t> base_IDs;
    emp::vector<size_t> vector_IDs;

    const emp::DataLayout & layout;

    std::string error_trait = "";
  public:
    TraitSet(const emp::DataLayout & in_layout) : layout(in_layout) { }
    ~TraitSet() = 0;

    void Clear() {
      base_names.resize(0); vector_names.resize(0);
      base_IDs.resize(0); vector_IDs.resize(0);
    }

    /// Add any number of traits, separated by commas.
    bool AddTraits(const std::string & in_names) {
      auto names = emp::slice(in_names, ',');
      for (const std::string & name : names) {
        if (!layout.HasName(name)) {
          error_trait = name;
          return false;
        }
        size_t id = layout.GetID(name);
        if (layout.IsType<T>(id)) {
          base_names.push_back(name);
          base_IDs.push_back(id);
        }
        else if (layout.IsType<emp::vector<T>>(id)) {
          vector_names.push_back(name);
          vector_IDs.push_back(id)l
        }
        else {
          error_trait = name;
          return false;
        }
      }
      return true;
    }

    /// Add groups of traits; each string can have multiple trait names separated by commas.
    template <typename... Ts>
    bool AddTraits(const std::string & in_names, const std::string & next_names, Ts &... extras) {
      return AddTraits(in_names) && AddTraits(next_names, extras...);
    }
  };

}

#endif
