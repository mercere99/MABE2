/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  TraitSet.hpp
 *  @brief A collection of traits with the same type (or collections of that type).
 *
 *  A TraitSet is used to keep track of a collection of related traits in a module.
 * 
 *  @CAO: Should this class be moved into Empirical proper?
 */

#ifndef MABE_TRAIT_SET_H
#define MABE_TRAIT_SET_H

#include <string>

#include "emp/base/vector.hpp"
#include "emp/data/DataMap.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/datastructs/vector_utils.hpp"

namespace mabe {

  template <typename T>
  class TraitSet {
  private:
    emp::vector<std::string> base_names;
    emp::vector<std::string> vector_names;
    emp::vector<size_t> base_IDs;
    emp::vector<size_t> vector_IDs;
    emp::vector<size_t> vec_sizes;

    emp::Ptr<const emp::DataLayout> layout;

    size_t num_values = 0;
    std::string error_trait = "";
  public:
    TraitSet() : layout(nullptr) { }
    TraitSet(const emp::DataLayout & in_layout) : layout(&in_layout) { }
    ~TraitSet() = default;

    emp::vector<std::string> GetNames() const { return emp::Concat(base_names, vector_names); }

    const emp::DataLayout & GetLayout() const { return *layout; }
    void SetLayout(const emp::DataLayout & in_layout) { layout = &in_layout; }

    void Clear() {
      base_names.resize(0); vector_names.resize(0);
      base_IDs.resize(0); vector_IDs.resize(0); vec_sizes.resize(0);
      num_values = 0;
    }

    /// Add any number of traits, separated by commas.
    bool AddTraits(const std::string & in_names) {
      emp_assert(!layout.IsNull());

      num_values = 0;
      auto names = emp::slice(in_names, ',');
      for (const std::string & name : names) {
        if (!layout->HasName(name)) {
          error_trait = name;
          return false;
        }
        size_t id = layout->GetID(name);
        if (layout->IsType<T>(id)) {
          base_names.push_back(name);
          base_IDs.push_back(id);
        }
        else if (layout->IsType<emp::vector<T>>(id)) {
          vector_names.push_back(name);
          vector_IDs.push_back(id);
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

    /// Clear any existing traits and load in the ones provided.
    template <typename... Ts>
    bool SetTraits(Ts &... traits) {
      Clear();
      return AddTraits(traits...);
    }

    /// Total number of direct traits.
    size_t GetNumBaseTraits() const { return base_IDs.size(); }

    /// Total number of traits that are collections of values in vectors.
    size_t GetNumVectorTraits() const { return vector_IDs.size(); }

    /// Get the total number of traits being monitored (regular values + vectors of values)
    size_t GetNumTraits() const { 
      return base_IDs.size() + vector_IDs.size();
    }

    /// Count the total number of individual values across all traits and store for future use.
    size_t CountValues(const emp::DataMap & dmap) {
      emp_assert(!layout.IsNull());
      emp_assert(dmap.HasLayout(*layout), "Attempting CountValues() on DataMap with wrong layout");

      num_values = base_IDs.size();
      vec_sizes.resize(vector_IDs.size());
      for (size_t i = 0; i < vector_IDs.size(); ++i) {
        const size_t id = vector_IDs[i];
        const size_t cur_size = dmap.Get<emp::vector<T>>(id).size();
        num_values += cur_size;
        vec_sizes[i] = cur_size;
      }
      return num_values;
    }

    /// Get last calculated count of values; set to zero if count not up to date.
    size_t GetNumValues() const { return num_values; }

    /// Get all associated values out of a data map and place them into a provided vector.
    void GetValues(const emp::DataMap & dmap, emp::vector<T> & out) {
      emp_assert(!layout.IsNull());

      // Make sure we have the right amount of room for the values.
      out.resize(0);
      out.reserve(GetNumValues());

      // Collect the base values.
      for (size_t trait_id : base_IDs) {
        out.push_back( dmap.Get<T>(trait_id) );
      }

      // Collect the vector values.
      for (size_t trait_id : vector_IDs) {
        const emp::vector<T> & cur_vec = dmap.Get<emp::vector<T>>(trait_id);
        out.insert(out.end(), cur_vec.begin(), cur_vec.end());
      }
    }

    /// Copy associated values from data map to a provided vector, only for positions specified;
    /// all other positions are 0.0.
    void GetValues(const emp::DataMap & dmap,
                   emp::vector<T> & out,
                   const emp::vector<size_t> & ids_used) {
      emp_assert(!layout.IsNull());

      // Make sure we have the right amount of room for the values.
      out.resize(0);
      out.resize(GetNumValues(), 0.0);

      for (size_t id : ids_used) {
        // If the ID is for a base trait, grab it.
        if (id < base_IDs.size()) {
          const size_t trait_id = base_IDs[id];
          out[id] = dmap.Get<T>(trait_id);
        }

        // Otherwise it must be from a vector.
        else {
          size_t vector_pos = id - base_IDs.size();  // Adjust id to be in range.

          // Step through the vectors to find the one with this index.
          size_t vid = 0;
          bool found = false;
          while (vid < vector_IDs.size()) {
            const size_t trait_id = vector_IDs[vid];
            const emp::vector<T> & cur_vec = dmap.Get<emp::vector<T>>(trait_id);
            if (vector_pos < cur_vec.size()) {
              out[id] = cur_vec[vector_pos];
              found = true;
              break;
            }
            vector_pos -= cur_vec.size();
            vid++;
          }
          emp_assert(found, "PROBLEM!  TraitSet ran out of vectors without finding trait id.");
        }
      }
    }

    /// Get a value at the specified index of this map.
    T GetIndex(const emp::DataMap & dmap, size_t value_index) const {
      emp_assert(value_index < num_values, value_index, num_values);

      // If this is a regular trait, return its value.
      if (value_index < base_IDs.size()) return dmap.Get<T>(value_index);

      // If it's a vector trait, look it up.
      value_index -= base_IDs.size();
      size_t vec_index = 0;
      while (value_index >= vec_sizes[vec_index]) {
        value_index -= vec_sizes[vec_index];
        vec_index++;
      }
      return dmap.Get<emp::vector<T>>(vector_IDs[vec_index])[value_index];
    }

    void PrintDebug() const {
      std::cout << "Base names: " << emp::ToString(base_names) << std::endl;
      std::cout << "Vector names: " << emp::ToString(vector_names) << std::endl;
    }
  };

}

#endif
