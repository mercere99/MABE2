/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2021.
 *
 *  @file  Collection.hpp
 *  @brief A collection of organisms or whole populations; not owner.
 *
 *  While organisms must be managed by Population objects, collections are an easy way
 *  to represent and manipulate groups of organisms (by their position).  Organisms can be
 *  added individually or as whole populations.
 * 
 *  Internally, a Collection is represented by a map; keys are pointers to the included Populations
 *  and values are a PopInfo class (a flag for "do we included the whole population" and a
 *  BitVector indicating the positions that are included if not the whole population).
 * 
 *  A CollectionIterator will track the current population being iterated through, and the position
 *  currently indicated.  When an iterator reached the end, it's population pointer is set to 
 *  nullptr.
 */

#ifndef MABE_COLLECTION_H
#define MABE_COLLECTION_H

#include <set>
#include <string>
#include <sstream>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/bits/BitVector.hpp"

#include "Population.hpp"

namespace mabe {

  class Collection;

  // A curtiously recursive template to create a base class for all collection iterators.
  template <typename DERIVED_T, typename ORG_T, typename COLLECTION_T=Collection>
  class CollectionIterator_Interface
    : public OrgIterator_Interface<DERIVED_T, ORG_T, emp::match_const_t<Population,COLLECTION_T>>
  {
    friend Collection;
  protected:
    emp::Ptr<COLLECTION_T> collection_ptr = nullptr;

    using base_t =
      OrgIterator_Interface<DERIVED_T, ORG_T, emp::match_const_t<Population,COLLECTION_T>>;
    using this_t = CollectionIterator_Interface<DERIVED_T, ORG_T>;

    void IncPosition() override;
    void DecPosition() override;
    void ShiftPosition(int shift=1) override;
    void ToBegin() override;
    void ToEnd() override;

  public:
    /// Constructor where you can optionally supply population pointer and position.
    CollectionIterator_Interface(emp::Ptr<COLLECTION_T> _col=nullptr, size_t _pos=0);

    /// Constructor where you can optionally supply population pointer and position.
    CollectionIterator_Interface(emp::Ptr<COLLECTION_T> _col, emp::Ptr<Population> pop, size_t _pos=0);

    /// Supply Collection by reference instead of pointer.
    CollectionIterator_Interface(COLLECTION_T & col, size_t _pos=0);

    /// Copy constructor
    template <typename T2, typename ORG_T2>
    CollectionIterator_Interface(const CollectionIterator_Interface<T2, ORG_T2> &);
 
    /// Copy operator
    template <typename T2, typename ORG_T2>
    CollectionIterator_Interface & operator=(const CollectionIterator_Interface<T2, ORG_T2> & in);
  };

  class CollectionIterator
    : public CollectionIterator_Interface<CollectionIterator, Organism> {
  public:
    using base_t = CollectionIterator_Interface<CollectionIterator, Organism>;

    /// Constructor where you can optionally supply population pointer and position.
    CollectionIterator(emp::Ptr<Collection> _col=nullptr, size_t _pos=0) : base_t(_col, _pos) {}

    /// Constructor where you can optionally supply population pointer and position.
    CollectionIterator(emp::Ptr<Collection> _col, emp::Ptr<Population> pop, size_t _pos=0)
      : base_t(_col, pop, _pos) {}

    /// Supply Collection by reference instead of pointer.
    CollectionIterator(Collection & col, size_t _pos=0) : base_t(col, _pos) {}

    /// Copy constructor
    CollectionIterator(const CollectionIterator & in) : base_t(in) {}
  };

  class ConstCollectionIterator
    : public CollectionIterator_Interface<ConstCollectionIterator, const Organism, const Collection>
  {
  public:
    using base_t =
      CollectionIterator_Interface<ConstCollectionIterator, const Organism, const Collection>;

    /// Constructor where you can optionally supply population pointer and position.
    ConstCollectionIterator(emp::Ptr<const Collection> _col=nullptr, size_t _pos=0) : base_t(_col, _pos) {}

    /// Constructor where you can optionally supply population pointer and position.
    ConstCollectionIterator(emp::Ptr<const Collection> _col, emp::Ptr<Population> pop, size_t _pos=0)
      : base_t(_col, pop, _pos) {}

    /// Supply Collection by reference instead of pointer.
    ConstCollectionIterator(const Collection & col, size_t _pos=0) : base_t(col, _pos) {}

    /// Copy constructors
    ConstCollectionIterator(const CollectionIterator & in) : base_t(in) {}
    ConstCollectionIterator(const ConstCollectionIterator & in) : base_t(in) {}
  };


  class Collection : public OrgContainer {
  private:
    using pop_ptr_t = emp::Ptr<mabe::Population>;
    using const_pop_ptr_t = emp::Ptr<const mabe::Population>;

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
        return (size_t) pos_set.FindOne();
      }

      /// Identify the next position after the one provided.  If there is no next position,
      /// return a value >= population size.
      size_t GetNextPos(size_t pos) const {
        if (full_pop) return ++pos;
        return (size_t) pos_set.FindOne(pos+1);
      }

      /// Remap an ID from the collection to a population position.
      size_t GetPos(size_t org_id) {
        if (full_pop) return org_id;

        size_t cur_pos = 0;
        while(org_id--) {
          cur_pos = pos_set.FindOne(cur_pos+1);
        }
        return cur_pos;
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

    // Link each population in the collection (by its pointer) to info about which organisms
    // are included.
    using pos_map_t = std::map<pop_ptr_t, PopInfo>;
    pos_map_t pos_map;

    // Helper Functions
    auto GetInfoIT(pop_ptr_t pop_ptr) {
      return pos_map.find(pop_ptr);
    }
    auto GetInfoIT(const_pop_ptr_t pop_ptr) {
      return pos_map.find(pop_ptr.ConstCast<mabe::Population>());
    }
    auto GetInfoIT(pop_ptr_t pop_ptr) const {
      return pos_map.find(pop_ptr);
    }
    auto GetInfoIT(const_pop_ptr_t pop_ptr) const {
      return pos_map.find(pop_ptr.ConstCast<mabe::Population>());
    }

    // Take an iterator that may be in an illegal state and restore it to a legal state.
    // Return whether it was originally valid.
    template <typename T>
    bool MakeValid(T & it) {
      const_pop_ptr_t cur_pop = it.PopPtr();  // Collect the current population pointer.
      if (cur_pop == nullptr) return true;    // This is an "end" iterator.

      auto info_it = GetInfoIT(cur_pop);      // Look up this population's info.

      // If we have an invalid population, jump to end and signal that it was invalid.
      if (info_it != pos_map.end()) {
        it.Set(nullptr, 0);
        return false;
      }

      // We now know we have a valid population.  Check if we are at a valid position.
      if (info_it->second.pos_set.Has(it.Pos())) return true;

      // Must move to a valid position, either in this population, another population, or end.
      // Find the position of the next organism from this population
      size_t next_pos = info_it->second.GetNextPos(it.Pos());

      // If this position is good, set it!
      if (next_pos < cur_pop->GetSize()) { it.SetPos(next_pos); return false; }

      // Otherwise advance to the first position in the next non-empty population OR end iterator.
      while (++info_it != pos_map.end() && info_it->second.GetSize(info_it->first) == 0);

      if (info_it == pos_map.end()) it.Set(nullptr, 0);           // No more populations!
      else it.Set(info_it->first, info_it->second.GetFirstPos()); // First position in next pop.

      return false;
    }

  public:
    Collection() = default;
    Collection(const Collection &) = default;
    Collection(Collection &&) = default;

    template <typename... Ts>
    Collection(Population & pop, Ts &&... extras) { Insert( pop, std::forward<Ts>(extras)... ); }

    template <typename... Ts>
    Collection(OrgPosition pos, Ts &&... extras) { Insert( pos, std::forward<Ts>(extras)... ); }

    ~Collection() { }

    Collection & operator=(const Collection &) = default;
    Collection & operator=(Collection &&) = default;

    using iterator_t = CollectionIterator;
    using const_iterator_t = ConstCollectionIterator;

    /// Calculate the total number of positions represented in this collection.
    size_t GetSize() const noexcept override {
      size_t count = 0;
      for (auto [pop_ptr, pop_info] : pos_map) {
        count += pop_info.GetSize(pop_ptr);
      }
      return count;
    }

    Organism & At(size_t org_id) override {
      for (auto [pop_ptr, pop_info] : pos_map) {
        if (org_id < pop_info.GetSize(pop_ptr)) {
          size_t pos = pop_info.GetPos(org_id);
          return pop_ptr->At(pos);
        }
        org_id -= pop_info.GetSize(pop_ptr);
      }

      // @CAO Should report error to user!
      emp_error("Trying to find org id out of range for a collection.");
      return pos_map.begin()->first->At(0); // Return the first organism since out of range.
    }

    const Organism & At(size_t org_id) const override {
      for (auto [pop_ptr, pop_info] : pos_map) {
        if (org_id < pop_info.GetSize(pop_ptr)) {
          size_t pop_id = pop_info.GetPos(org_id);
          return pop_ptr->At(pop_id);
        }
        org_id -= pop_info.GetSize(pop_ptr);
      }

      // @CAO Should report error to user!
      emp_error("Trying to find org id out of range for a collection.");
      return pos_map.begin()->first->At(0); // Return the first organism since out of range.
    }

    Organism & operator[](size_t org_id) { return At(org_id); }
    const Organism & operator[](size_t org_id) const { return At(org_id); }

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

    const_pop_ptr_t GetFirstPop() const {
      if (pos_map.size() == 0) return nullptr;
      else return pos_map.begin()->first;
    }

    template <typename T>
    void IncPosition(T & it) const {
      const_pop_ptr_t cur_pop = it.PopPtr();
      auto info_it = GetInfoIT(cur_pop);

      // Make sure that the current population was found!  This check will fail if either
      // we are already at the end OR it is pointed to a population not in the collection.
      emp_assert(info_it != pos_map.end(),
                 "Invalid start position before collection iterator is incremented");

      // Find the position of the next organism from this population
      size_t next_pos = info_it->second.GetNextPos(it.Pos());

      // If this position is good, set it!
      if (next_pos < cur_pop->GetSize()) it.SetPos(next_pos);

      // Otherwise advance to the next population,
      else {
        // Advance population pointer; If we are at a population and it is empty, keep advancing!
        while (++info_it != pos_map.end() && info_it->second.GetSize(info_it->first) == 0);
        if (info_it == pos_map.end()) it.Set(nullptr, 0);           // No more populations!
        else it.Set(info_it->first, info_it->second.GetFirstPos());
      }
    }

    template <typename T>
    void DecPosition(T & /* it */) const {
      emp_error("DecPosition() not yet implemented for CollectionIterator.");
    }
    template <typename T>
    void ShiftPosition(T & /* it */, int /* shift */) const {
      emp_error("ShiftPosition() not yet implemented for CollectionIterator.");
    }

    CollectionIterator begin() { return CollectionIterator(this); }
    CollectionIterator end() { return CollectionIterator(this, nullptr); }
    ConstCollectionIterator begin() const { return ConstCollectionIterator(this); }
    ConstCollectionIterator end() const { return ConstCollectionIterator(this, nullptr); }

    /// Add a Population to this collection.
    template <typename... Ts>
    Collection & Insert(Population & pop, Ts &&... extras) {
      pos_map[&pop].full_pop = true;
      return Insert( std::forward<Ts>(extras)... );
    }

    /// Add an organism (by position!)
    template <typename... Ts>
    Collection & Insert(OrgPosition pos, Ts &&... extras) {
      pos_map[pos.PopPtr()].InsertPos(pos.Pos());
      return Insert( std::forward<Ts>(extras)... );
    }

    /// Add a whole other collection.
    template <typename... Ts>
    Collection & Insert(const Collection & in_collection, Ts &&... extras) {
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

      return Insert( std::forward<Ts>(extras)... );
    }

    /// Base case...
    Collection & Insert() { return *this; }

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
        for (int pos = pos_set.FindOne(); pos != -1; pos = pos_set.FindOne(pos+1)) {
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

    /// Reduce to the intersection with another collection.
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
  //  Implementations of CollectionIterator member functions
  // -------------------------------------------------------
  
  template <typename DERIVED_T, typename ORG_T, typename COLLECTION_T>
  void CollectionIterator_Interface<DERIVED_T, ORG_T, COLLECTION_T>::IncPosition() {
    emp_assert(collection_ptr);
    collection_ptr->IncPosition(*this);
  }
  template <typename DERIVED_T, typename ORG_T, typename COLLECTION_T>
  void CollectionIterator_Interface<DERIVED_T, ORG_T, COLLECTION_T>::DecPosition() {
    emp_assert(collection_ptr);
    collection_ptr->DecPosition(*this);
  }
  template <typename DERIVED_T, typename ORG_T, typename COLLECTION_T>
  void CollectionIterator_Interface<DERIVED_T, ORG_T, COLLECTION_T>::ShiftPosition(int shift) {
    emp_assert(collection_ptr);
    collection_ptr->ShiftPosition(*this, shift);
  }
  template <typename DERIVED_T, typename ORG_T, typename COLLECTION_T>
  void CollectionIterator_Interface<DERIVED_T, ORG_T, COLLECTION_T>::ToBegin() {
    emp_assert(collection_ptr);
    *this = collection_ptr->begin();
  }
  template <typename DERIVED_T, typename ORG_T, typename COLLECTION_T>
  void CollectionIterator_Interface<DERIVED_T, ORG_T, COLLECTION_T>::ToEnd() {
    emp_assert(collection_ptr);
    *this = collection_ptr->end();
  }

  /// Constructor where you can optionally supply population pointer and position.
  template <typename DERIVED_T, typename ORG_T, typename COLLECTION_T>
  CollectionIterator_Interface<DERIVED_T, ORG_T, COLLECTION_T>
    ::CollectionIterator_Interface(emp::Ptr<COLLECTION_T> _col, size_t _pos)
    : base_t(_col->GetFirstPop(), _pos), collection_ptr(_col)
  {
    // Make sure that this iterator is actually valid.  If not, move to next position.
    if (base_t::IsValid() == false) IncPosition();
  }

  /// Constructor where you can optionally supply population pointer and position.
  template <typename DERIVED_T, typename ORG_T, typename COLLECTION_T>
  CollectionIterator_Interface<DERIVED_T, ORG_T, COLLECTION_T>
    ::CollectionIterator_Interface(emp::Ptr<COLLECTION_T> _col, emp::Ptr<Population> pop, size_t _pos)
    : base_t(pop, _pos), collection_ptr(_col)
  {
  }
}

#endif
