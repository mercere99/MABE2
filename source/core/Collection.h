/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  Collection.h
 *  @brief A collection of organisms or whole populations; not owner.
 *
 *  While organisms must be managed by Population objects, collections are an easy way
 *  to represent and manipulate groups of organisms (by their position).  Organisms can be
 *  added individully or as whole populations.
 * 
 *  Internally, a Collection is represented by a map with keys of Population pointers and
 *  values of a BitVector indicating the positions in those populations that are included.
 */

#ifndef MABE_COLLECTION_H
#define MABE_COLLECTION_H

#include <set>
#include <string>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/BitVector.h"

#include "Population.h"

namespace mabe {

  class Collection;

  class CollectionIterator : public OrgIterator_Interface<CollectionIterator> {
    friend Collection;
  protected:
    emp::Ptr<Collection> collection_ptr = nullptr;

    using base_t = OrgIterator_Interface<CollectionIterator>;

    void IncPosition() override {
      emp_assert(collection_ptr);
      collection_ptr->IncPosition(*this);
    }
    void DecPosition() override {
      emp_assert(collection_ptr);
      collection_ptr->DecPosition(*this);
    }
    void ShiftPosition(int shift=1) override {
      emp_assert(collection_ptr);
      collection_ptr->ShiftPosition(*this, shift);
    }
    void ToBegin() override {
      emp_assert(collection_ptr);
      *this = collection_ptr->begin();
    }
    void ToEnd() override {
      emp_assert(collection_ptr);
      *this = collection_ptr->end();
    }
    void MakeValid() override {
      if (collection_ptr) collection_ptr->MakeValid(*this);
    }

  public:
    /// Constructor where you can optionally supply population pointer and position.
    CollectionIterator(emp::Ptr<Collection> _col=nullptr, size_t _pos=0)
      : base_t(_col->GetFirstPop(), _pos), collection_ptr(_col) { ; }

    /// Constructor where you can optionally supply population pointer and position.
    CollectionIterator(emp::Ptr<Collection> _col, emp::Ptr<Population> pop, size_t _pos=0)
      : base_t(pop, _pos), collection_ptr(_col) { ; }

    /// Supply Population by reference instead of pointer.
    CollectionIterator(Collection & col, size_t _pos=0) : CollectionIterator(&col, _pos) {}

    /// Copy constructor
    CollectionIterator(const CollectionIterator &) = default;

    /// Copy operator
    CollectionIterator & operator=(const CollectionIterator & in) = default;
  };


  class Collection {
  private:
    using pop_ptr_t = emp::Ptr<mabe::Population>;

    /// Information about a single population in this collection.
    struct PopInfo {
      bool full_pop = false;   ///< Should we use the full population?
      emp::BitVector pos_set;  ///< Which positions are we using for this population?

      // Identify how many positions we have.
      size_t GetSize(pop_ptr_t pop_ptr) const {
        if (full_pop) return pop_ptr->GetSize();
        return pos_set.CountOnes();
      }

      // Insert a single position into the pos_set.
      void InsertPos(size_t pos) {
        // Make sure we have room for this position and then set it.
        if (pos_set.GetSize() <= pos) pos_set.Resize(pos+1);
        pos_set.Set(pos);
      }

      // Shift this population to using the pos_set.
      void RemoveFull(pop_ptr_t pop_ptr) {
        if (!full_pop) return;
          pos_set.Resize(pop_ptr->GetSize());
          pos_set.SetAll();
          full_pop = false;
      }
    };

    using map_t = std::map<pop_ptr_t, PopInfo>;
    map_t pos_map;

  public:
    Collection() = default;
    Collection(const Collection &) = default;
    Collection(Collection &&) = default;
    Collection(const Population & pop) { Insert(pop); }
    ~Collection() { }

    Collection & operator=(const Collection &) = default;
    Collection & operator=(Collection &&) = default;

    /// Calculation the total number of positions represented in this collection.
    size_t GetSize() const {
      size_t count = 0;
      for (auto [pop_ptr, pop_info] : pos_map) {
        count += pop_info.GetSize(pop_ptr);
      }
      return count;
    }

    pop_ptr_t GetFirstPop() {
      if (pos_map.size() == 0) return nullptr;
      else return pos_map.begin()->first;
    }

    void IncPosition(CollectionIterator & it) const { }
    void DecPosition(CollectionIterator & it) const {
      emp_error("DecPosition() not yet implemented for CollectionIterator.");
    }
    void ShiftPosition(CollectionIterator & it, int shift) const {
      emp_error("ShiftPosition() not yet implemented for CollectionIterator.");
    }
    void MakeValid(CollectionIterator & it) const {
      // @CAO Implement this?
    }

    CollectionIterator begin() const { return CollectionIterator(this); }
    CollectionIterator end() const { return CollectionIterator(this, nullptr); }

    /// Add a Population to this collection.
    Collection &  Insert(const Population & pop) {
      pos_map[&pop].full_pop = true;
      return *this;
    }

    /// Add an organism (by position!)
    Collection & Insert(OrgPosition pos) {
      pos_map[pos.PopPtr()].InsertPos(pos.Pos());
      return *this;
    }

    /// Add a whole other collection.
    Collection & Insert(const Collection & in_collection) {
      for (auto & [pop_ptr, in_pop_info] : in_collection.pos_map) {
        PopInfo & pop_info = pos_map[pop_ptr];

        if (pop_info.full_pop) continue;  // This population is already full.

        // If we're adding a full population, do so.
        if (in_pop_info.full_pop) { pop_info.full_pop = true; continue; }

        // Otherwise add just the entries we need to.

        emp::BitVector & pos_set = pop_info.pos_set;
        emp::BitVector in_pos_set = in_pop_info.pos_set;

        // First, make sure both position sets are the same size.
        if (in_pos_set.GetSize() < pos_set.GetSize()) {
          in_pos_set.Resize(pos_set.GetSize());
        }
        else if (pos_set.GetSize() < in_pos_set.GetSize()) {
          pos_set.Resize(in_pos_set.GetSize());
        }

        // Use 'OR' to join the sets.
        pos_set |= in_pos_set;
      }

      return *this;
    }

    // @CAO: Add:
    // * Remove()  - works with position or population (or another collection?)
    // * Has() - position
    // * Filter() - takes a function and only keeps positions that return true.
    //   (Filter could even have a three function version that takes a second collection and
    //   indicated what to do with those just in the first collection, just in the second,
    //   or in both.)

    /// Remove all empty positions from this collection.
    Collection & RemoveEmpty() {
      for (auto & [pop_ptr, pop_info] : pos_map) {
        emp::BitVector & pos_set = pop_info.pos_set;

        pop_info.RemoveFull(pop_ptr); // Make sure this population isn't full.

        // Scan through organisms, removing inclusion of those that are empty.
        for (int pos = pos_set.FindBit(); pos != -1; pos = pos_set.FindBit(pos)) {
          if (!pop_ptr->IsOccupied((size_t) pos)) pos_set.Set(pos,false);
        }
      }
    }

    /// Merge this collection with another collection.
    Collection & operator |= (const Collection & collection2) {
      return Insert(collection2);
    }

    /// Reduce to the intersection with another colleciton.
    Collection & operator &= (const Collection & in_collection) {
      auto cur_it = pos_map.begin();
      auto in_it = in_collection.pos_map.begin();

      // Step through both iterators dealing appropriately with populations.
      while (cur_it != pos_map.end() && in_it != in_collection.pos_map.end()) {
        // If the 'in' iterator is smaller, ignore this population and move on.
        if (in_it->first < cur_it->first) { in_it++; continue; }
        
        // If the current iterator is smaller, delete the current population (not in intersection)
        if (cur_it->first < in_it->first) { cur_it = pos_map.erase(cur_it); continue; }

        // Otherwise populations must be the same!  If second pop is full, keep first as is!
        if (!in_it->second.full_pop) {
          cur_it->second.RemoveFull(cur_it->first);         // Shift first pop to individuals
          cur_it->second.pos_set &= in_it->second.pos_set;  // Now pick out the intersection.
        }

        // Move on to the next populations.
        cur_it++;
        in_it++;
      }

      // Remove any extra populations that didn't match in the in_collection.
      while (cur_it != pos_map.end()) cur_it = pos_map.erase(cur_it);

      return *this;
    }

  };

}

#endif
