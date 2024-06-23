/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2024.
 *
 *  @file  TraitSet.hpp
 *  @brief A collection of traits with the same type (or collections of that type).
 *
 *  A TraitSet is used to keep track of a collection of related traits in a module.
 * 
 *  For example, this class is used inside of Lexicase selection to track the group of
 *  traits under consideration during optimization.
 * 
 *  @CAO: Should this class be moved into Empirical proper?
 */

#ifndef MABE_TRAIT_SET_H
#define MABE_TRAIT_SET_H

#include "emp/base/vector.hpp"
#include "emp/data/DataMap.hpp"
#include "emp/meta/TypeID.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/tools/String.hpp"

namespace mabe {

  template <typename T>
  class TraitSet {
  private:
    // Each entry in a trait set can be a single trait (BASE), a series of sequential traits
    // (MULTI), or an emp::vector of the trait type (VECTOR)
    enum TraitType { BASE=0, MULTI=1, VECTOR=2 };

    // When tracking a trait, we care about its type (see previous), where it is in the layout (id)
    // how many trait values we are talking about (count), and how many total values are up to
    // this point (cum_count) to facilitate searches.
    struct TraitData {
      TraitType type;
      size_t id;
      size_t count;
      size_t cum_count=0;  // How many total sites used until the end of this one?
      TraitData(TraitType _t=BASE, size_t _id=0, size_t _c=1) : type(_t), id(_id), count(_c) { }
    };

    emp::vector<emp::String> trait_names;
    emp::vector<TraitData> trait_data;

    emp::Ptr<const emp::DataLayout> layout;  // Layout for the DataMaps that we will access.

    size_t num_values = 0;
    emp::String error_trait = "";
    
  public:
    TraitSet() : layout(nullptr) { }
    TraitSet(const emp::DataLayout & in_layout) : layout(&in_layout) { }
    ~TraitSet() = default;

    emp::vector<emp::String> GetNames() const { return trait_names; }

    const emp::DataLayout & GetLayout() const { return *layout; }
    void SetLayout(const emp::DataLayout & in_layout) { layout = &in_layout; }

    void Clear() {
      trait_names.resize(0);
      trait_data.resize(0);
      num_values = 0;
    }

    /// Add a single trait.
    bool AddTrait(const emp::String & name) {
      if (!layout->HasName(name)) {
        error_trait = name;
        return false;
      }

      trait_names.push_back(name);
      const size_t id = layout->GetID(name);      
      const size_t count = layout->GetCount(id);
      if (layout->IsType<T>(id)) {
        if (count == 1) trait_data.emplace_back(TraitType::BASE, id, 1);
        else trait_data.emplace_back(TraitType::MULTI, id, count);
      }
      else if (layout->IsType<emp::vector<T>>(id) && count == 1) {
        trait_data.emplace_back(TraitType::VECTOR, id, 1);
      }
      else {
        error_trait = name;
        return false;
      }
      return true;
    }

    /// Add any number of traits, separated by commas.
    bool AddTraits(const emp::String & in_names) {
      emp_assert(!layout.IsNull());

      auto names = in_names.Slice(",");
      for (const emp::String & name : names) {
        if (AddTrait(name) == false) return false;
      }
      return true;
    }

    /// Add groups of traits; each string can have multiple trait names separated by commas.
    template <typename... Ts>
    bool AddTraits(const emp::String & in_names, const emp::String & next_names, Ts &... extras) {
      return AddTraits(in_names) && AddTraits(next_names, extras...);
    }

    /// Clear any existing traits and load in the ones provided.
    template <typename... Ts>
    bool SetTraits(Ts &... traits) {
      Clear();
      return AddTraits(traits...);
    }

    /// Get the total number of traits being monitored (regular values + vectors of values)
    size_t GetNumTraits() const { 
      return trait_data.size();
    }

    /// Count the total number of individual values across all traits and store for future use.
    size_t CountValues(const emp::DataMap & dmap) {
      emp_assert(!layout.IsNull());
      emp_assert(dmap.HasLayout(*layout), "Attempting CountValues() on DataMap with wrong layout");

      num_values = 0;
      for (TraitData & data : trait_data) {
        switch (data.type) {
          case TraitType::BASE: ++num_values; break;
          case TraitType::MULTI: num_values += data.count; break;
          case TraitType::VECTOR: 
            data.count = dmap.Get<emp::vector<T>>(data.id).size();
            num_values += data.count;
        }
        data.cum_count = num_values;
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

      // Loop through collecting values.
      for (TraitData & data : trait_data) {
        switch (data.type) {
        case TraitType::BASE:
          out.push_back( dmap.Get<T>(data.id) );
          break;
        case TraitType::MULTI: {
          std::span<const T> cur_span = dmap.Get<T>(data.id, data.count);
          out.insert(out.end(), cur_span.begin(), cur_span.end());
        } break;
        case TraitType::VECTOR: {
          const emp::vector<T> & cur_vec = dmap.Get<emp::vector<T>>(data.id);
          out.insert(out.end(), cur_vec.begin(), cur_vec.end());
        } break;
        }
      }
    }

    /// Copy associated values from data map to a provided vector, only for positions specified;
    /// all other positions are 0.0.
    void GetValues(const emp::DataMap & dmap,
                   emp::vector<T> & out,
                   const emp::vector<size_t> & ids_used) {
      emp_assert(!layout.IsNull());
      emp_assert(std::is_sorted(ids_used.begin(), ids_used.end())); // Requested values should be in sorted order.

      // Make sure we have the right amount of room for the values, with non-used ones set to zero.
      out.resize(0);
      out.resize(GetNumValues(), 0.0);

      size_t trait_id = 0;
      size_t offset = 0;
      for (const size_t id : ids_used) {
        while (id >= trait_data[trait_id].cum_count) {
          offset = trait_data[trait_id].cum_count;
          ++trait_id;
          emp_assert(trait_id < trait_data.size(),
                     "PROBLEM!  TraitSet ran out of vectors without finding trait id.");
        }
        switch (trait_data[trait_id].type) {
          case TraitType::BASE:
            emp_assert(id == offset, id, offset);
            out[id] = dmap.Get<T>(trait_id);
            break;
          case TraitType::MULTI: {
            std::span<const T> cur_span = dmap.Get<T>(trait_id, trait_data[trait_id].count);
            out[id] = cur_span[id-offset];
          } break;
          case TraitType::VECTOR: {
            const emp::vector<T> & cur_vec = dmap.Get<emp::vector<T>>(trait_id);
            out[id] = cur_vec[id-offset];
          } break;
        }
      }

    }

    /// Get a value at the specified index of this map.
    //  @TODO: This could be sped up using binary search.
    T GetIndex(const emp::DataMap & dmap, size_t value_index) const {
      emp_assert(value_index < num_values, value_index, num_values);

      size_t trait_id = 0;
      while (value_index >= trait_data[trait_id].cum_count) ++trait_id;

      const size_t offset = (trait_id==0) ? 0 : trait_data[trait_id-1].cum_count;
      const TraitData & data = trait_data[trait_id];

      switch (data.type) {
        case TraitType::BASE:   return dmap.Get<T>(data.id);
        case TraitType::MULTI:  return dmap.Get<T>(data.id, data.count)[value_index - offset];
        case TraitType::VECTOR: return dmap.Get<emp::vector<T>>(data.id)[value_index - offset];
      }

      return T{};
    }


    void PrintDebug() const {
      std::cout << "Trait names: " << emp::ToString(trait_names) << std::endl;
    }
  };

}

#endif
