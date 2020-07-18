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
#include <sstream>

#include "base/Ptr.h"
#include "base/vector.h"
#include "tools/BitVector.h"

#include "Population.h"

namespace mabe {

  class Collection;

  class CollectionIterator : public OrgIterator_Interface<CollectionIterator, Organism, Population> {
    friend Collection;
  protected:
    emp::Ptr<Collection> collection_ptr = nullptr;

    using base_t = OrgIterator_Interface<CollectionIterator, Organism, Population>;

    void IncPosition() override;
    void DecPosition() override;
    void ShiftPosition(int shift=1) override;
    void ToBegin() override;
    void ToEnd() override;
    void MakeValid() override;

  public:
    /// Constructor where you can optionally supply population pointer and position.
    CollectionIterator(emp::Ptr<Collection> _col=nullptr, size_t _pos=0);

    /// Constructor where you can optionally supply population pointer and position.
    CollectionIterator(emp::Ptr<Collection> _col, emp::Ptr<Population> pop, size_t _pos=0);

    /// Supply Population by reference instead of pointer.
    CollectionIterator(Collection & col, size_t _pos=0);

    /// Copy constructor
    CollectionIterator(const CollectionIterator &) = default;

    /// Copy operator
    CollectionIterator & operator=(const CollectionIterator & in) = default;
  };


  class Collection : public OrgContainer {
  private:
    using pop_ptr_t = emp::Ptr<mabe::Population>;

    /// Information about a single population in this collection.
    struct PopInfo {
      bool full_pop = false;   ///< Should we use the full population?
      emp::BitVector pos_set;  ///< Which positions are we using for this population?

      /// Identify how many positions we have.
      size_t GetSize(pop_ptr_t pop_ptr) const {
        if (full_pop) return pop_ptr->GetSize();
        return pos_set.CountOnes();
      }

      /// Return the first legal position in the population (or 0 if none exist, which
      /// should never happen!)
      size_t GetFirstPos() const {
        if (full_pop) return 0;
        return (size_t) pos_set.FindBit();
      }

      /// Identify the next position after the one provided.  If there is no next position,
      /// return a value >= population size.
      size_t GetNextPos(size_t pos) const {
        if (full_pop) return ++pos;
        return (size_t) pos_set.FindBit(pos+1);
      }

      /// Insert a single position into the pos_set.
      void InsertPos(size_t pos) {
        if (full_pop) return;
        // Make sure we have room for this position and then set it.
        if (pos_set.GetSize() <= pos) pos_set.Resize(pos+1);
        pos_set.Set(pos);
      }

      /// Shift this population to using the pos_set.
      void RemoveFull(pop_ptr_t pop_ptr) {
        if (!full_pop) return;
          pos_set.Resize(pop_ptr->GetSize());
          pos_set.SetAll();
          full_pop = false;
      }
    };

    std::map<pop_ptr_t, PopInfo> pos_map;

  public:
    Collection() = default;
    Collection(const Collection &) = default;
    Collection(Collection &&) = default;
    Collection(Population & pop) { Insert(pop); }
    ~Collection() { }

    Collection & operator=(const Collection &) = default;
    Collection & operator=(Collection &&) = default;

    using iterator_t = CollectionIterator;

    /// Calculation the total number of positions represented in this collection.
    size_t GetSize() const noexcept {
      size_t count = 0;
      for (auto [pop_ptr, pop_info] : pos_map) {
        count += pop_info.GetSize(pop_ptr);
      }
      return count;
    }

    bool HasPopulation(const mabe::Population & pop) const {
      return emp::Has(pos_map, (Population *) &pop);
    }

    bool HasPosition(OrgPosition & pos) const {
      auto info_it = pos_map.find(pos.PopPtr());
      return info_it != pos_map.end() &&
             (info_it->second.full_pop || info_it->second.pos_set.Has(pos.Pos()));
    }

    // Convert this Collection into a string that can be used in configuration files.  For example:
    //   main_pop,special_pop[0-99],next_pop
    std::string ToString() const {
      std::stringstream ss;
      bool first = true;
      for (auto [pop_ptr, pop_info] : pos_map) {
        if (first) first = false;
        else ss << ',';

        // Indicate the name of the next population.
        ss << pop_ptr->GetName();

        // If we're not taking the whole population, specify the positions to use.
        if (pop_info.full_pop == false) {
          ss << '[';
          pop_info.pos_set.PrintAsRange(ss);
          ss << ']';
        }
      }

      return ss.str();
    }

    pop_ptr_t GetFirstPop() {
      if (pos_map.size() == 0) return nullptr;
      else return pos_map.begin()->first;
    }

    void IncPosition(CollectionIterator & it) const {
      pop_ptr_t cur_pop = it.PopPtr();
      auto info_it = pos_map.find(cur_pop);
      emp_assert(info_it != pos_map.end());

      // Find the position of the next organism from this population
      size_t next_pos = info_it->second.GetNextPos(it.Pos());

      // If this position is good, set it!
      if (next_pos < cur_pop->GetSize()) it.SetPos(next_pos);

      // Otherwise advance to the next population,
      else {
        ++info_it;
        if (info_it == pos_map.end()) it.Set(nullptr, 0);           // No more populations!
        else it.Set(info_it->first, info_it->second.GetFirstPos());
      }
    }
    void DecPosition(CollectionIterator & it) const {
      emp_error("DecPosition() not yet implemented for CollectionIterator.");
    }
    void ShiftPosition(CollectionIterator & it, int shift) const {
      emp_error("ShiftPosition() not yet implemented for CollectionIterator.");
    }
    void MakeValid(CollectionIterator & it) const {
      // @CAO Implement this?
    }

    CollectionIterator begin() { return CollectionIterator(this); }
    CollectionIterator end() { return CollectionIterator(this, nullptr); }

    /// Add a Population to this collection.
    Collection & Insert(Population & pop) {
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

        // Make sure this population isn't full.  Even if there a no empty cells, the
        // remove empty command should limit us to only the existing positions.
        pop_info.RemoveFull(pop_ptr); 

        // Scan through organisms, removing inclusion of those that are empty.
        for (int pos = pos_set.FindBit(); pos != -1; pos = pos_set.FindBit(pos+1)) {
          if (!pop_ptr->IsOccupied((size_t) pos)) pos_set.Set(pos,false);
        }
      }

      return *this;
    }

    /// Produce a new collection limited to living organisms.
    Collection GetAlive() {
      Collection out(*this);
      out.RemoveEmpty();
      return out;
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

  // -------------------------------------------------------
  //  Implementations of CollectionItertor member functions
  // -------------------------------------------------------

    void CollectionIterator::IncPosition() {
      emp_assert(collection_ptr);
      collection_ptr->IncPosition(*this);
    }
    void CollectionIterator::DecPosition() {
      emp_assert(collection_ptr);
      collection_ptr->DecPosition(*this);
    }
    void CollectionIterator::ShiftPosition(int shift) {
      emp_assert(collection_ptr);
      collection_ptr->ShiftPosition(*this, shift);
    }
    void CollectionIterator::ToBegin() {
      emp_assert(collection_ptr);
      *this = collection_ptr->begin();
    }
    void CollectionIterator::ToEnd() {
      emp_assert(collection_ptr);
      *this = collection_ptr->end();
    }
    void CollectionIterator::MakeValid() {
      if (collection_ptr) collection_ptr->MakeValid(*this);
    }

    /// Constructor where you can optionally supply population pointer and position.
    CollectionIterator::CollectionIterator(emp::Ptr<Collection> _col, size_t _pos)
      : base_t(_col->GetFirstPop(), _pos), collection_ptr(_col)
    {      
    }

    /// Constructor where you can optionally supply population pointer and position.
    CollectionIterator::CollectionIterator(emp::Ptr<Collection> _col, emp::Ptr<Population> pop, size_t _pos)
      : base_t(pop, _pos), collection_ptr(_col)
    {
    }
}

#endif
