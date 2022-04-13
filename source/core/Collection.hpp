/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2022.
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
      bool is_mutable = false; ///< Are we allowed to change this population?
      emp::BitVector pos_set;  ///< Which positions are we using for this population?

      /// Identify how many positions we have.
      size_t GetSize(pop_ptr_t pop_ptr) const {
        if (full_pop) return pop_ptr->GetSize();
        emp_assert(pop_ptr->GetSize() == pop_set.GetSize());
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

        size_t cur_pos = pos_set.FindOne();
        while(org_id) {
          org_id--;
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
        if (!full_pop) return;              // Already not a full population.
        pos_set.Resize(pop_ptr->GetSize()); // Resize position set to have room for all positions.
        pos_set.SetAll();                   // Initially include all orgs.
        full_pop = false;                   // Record that pop is no longer officially full.
      }

      bool IsEmpty(pop_ptr_t pop_ptr) {
        if (full_pop) return pop_ptr->IsEmpty();
        size_t cur_pos = 0;
        while(cur_pos < pop_ptr->GetSize()) {
          cur_pos = pos_set.FindOne(cur_pos+1);
          if (!pop_ptr->IsEmpty()) return false;
        }
        return true;
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
    Collection(const Population & pop, Ts &&... extras) { Insert( pop, std::forward<Ts>(extras)... ); }

    template <typename... Ts>
    Collection(OrgPosition pos, Ts &&... extras) { Insert( pos, std::forward<Ts>(extras)... ); }

    ~Collection() { }

    Collection & operator=(const Collection &) = default;
    Collection & operator=(Collection &&) = default;

    using iterator_t = CollectionIterator;
    using const_iterator_t = ConstCollectionIterator;

    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("ADD_COLLECT",
        [](Collection & collect, Collection & in) -> Collection&
          { return collect.Insert(in); },
        "Merge another collection into this one."
      );
      info.AddMemberFunction("ADD_ORG",
        [](Collection & collect, Population & pop, size_t id) -> Collection&
          { return collect.Insert(pop.IteratorAt(id)); },
        "Add a single position to this collection."
      );
      info.AddMemberFunction("ADD_POP",
        [](Collection & collect, Population & pop) -> Collection& { return collect.Insert(pop); },
        "Add a whole population to this collection."
      );
      info.AddMemberFunction("CLEAR",
        [](Collection & collect) -> Collection& { return collect.Clear(); },
        "Remove all entries from this collection."
      );
      info.AddMemberFunction("HAS_ORG",
        [](Collection & collect, Population & pop, size_t id)
          { return collect.HasPosition(pop.IteratorAt(id)); },
        "Is the specified org position in this collection?"
      );
      info.AddMemberFunction("HAS_POP",
        [](Collection & collect, Population & pop) { return collect.HasPopulation(pop); },
        "Is the specified population in this collection?"
      );
      info.AddMemberFunction("SET_ORG",
        [](Collection & collect, Population & pop, size_t id) -> Collection&
          { return collect.Set(pop.IteratorAt(id)); },
        "Set this collection to be a single position."
      );
      info.AddMemberFunction("SET_POP",
        [](Collection & collect, Population & pop) -> Collection& { return collect.Set(pop); },
        "Set this collection to be a whole population."
      );
      info.AddMemberFunction("SIZE",
        [](Collection & collect) { return collect.GetSize(); },
        "Identify how many positions are in this collection."
      );
    }

    template <typename OUT_T, typename IN_T> static OUT_T MakeRValueFrom(IN_T && in) {
      static_assert(std::is_same<OUT_T, Collection>(),
                    "Internal error: type mis-match for MakeRValueFrom()");
      // using decay_T = std::decay_t<IN_T>;
      if constexpr (std::is_same<IN_T, EmplodeType&>()) {
        // Test if we are converting from a population!
        emp::Ptr<EmplodeType> in_ptr = &in;
        auto pop_ptr = in_ptr.DynamicCast<Population>();
        if (pop_ptr) return Collection(*pop_ptr);

        // Currently, no other EmplodeTypes to convert from...
      }
      // Conversion from string requires MABE controller...
      // else if constexpr (std::is_same<decay_T, std::string>()) {
      // }
      // Cannot convert from double.
      // else if constexpr (std::is_same<decay_T, double>()) {
      // }
      emp_error("Cannot convert provided input to requested RValue", emp::GetTypeID<OUT_T>());
      return *((OUT_T *) &in);
    }

    /// Calculate the total number of positions represented in this collection.
    size_t GetSize() const noexcept override {
      size_t count = 0;
      for (auto [pop_ptr, pop_info] : pos_map) {
        count += pop_info.GetSize(pop_ptr);
      }
      return count;
    }

    /// Determine if there are any (living) organisms in this collection.
    bool IsEmpty() const noexcept override {
      // If we find an organism in any population, return false; otherwise return true.
      for (auto [pop_ptr, pop_info] : pos_map) {
        if (!pop_info.IsEmpty(pop_ptr)) return false;
      }
      return true;
    }

    iterator_t IteratorAt(size_t org_id) { return iterator_t(*this, org_id); }
    const_iterator_t IteratorAt(size_t org_id) const { return const_iterator_t(*this, org_id); }
    const_iterator_t ConstIteratorAt(size_t org_id) const { return IteratorAt(org_id); }

    Organism & At(size_t org_id) override {
      for (auto [pop_ptr, pop_info] : pos_map) {
        const size_t pop_size = pop_info.GetSize(pop_ptr);

        // If the ID is in the current population, get it.
        if (org_id < pop_size) {
          size_t pos = pop_info.GetPos(org_id);
          emp_assert(pop_info.is_mutable == true,
            "Cannot use At() for const population in Collection; try ConstAt() or use const iterator.");
          return pop_ptr->At(pos);
        }

        // Move on to the next population, subtracting off orgs from this one.
        org_id -= pop_size;
      }

      emp::notify::Error("Trying to find org id out of range for a collection.");
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

      emp::notify::Error("Trying to find org id out of range for a collection.");
      return pos_map.begin()->first->At(0); // Return the first organism since out of range.
    }

    // Always return a constant organism.
    const Organism & ConstAt(size_t org_id) const { return At(org_id); }

    Organism & operator[](size_t org_id) { return At(org_id); }
    const Organism & operator[](size_t org_id) const { return At(org_id); }

    bool HasPopulation(const mabe::Population & pop) const {
      return emp::Has(pos_map, (Population *) &pop);
    }

    bool HasPosition(const OrgPosition & pos) const {
      auto info_it = pos_map.find(pos.PopPtr().ConstCast<Population>());
      return info_it != pos_map.end() &&
             (info_it->second.full_pop || info_it->second.pos_set.Has(pos.Pos()));
    }

    // Convert this Collection into a string that can be used in configuration files.  For example:
    //   main_pop,special_pop[0-99],next_pop
    std::string ToString() const override {
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
      emp_assert(pos_map.begin()->second.is_mutable == true,
        "Cannot use GetFirstPop() for const Population in Collection; try ConstGetFirstPop().");
      return pos_map.begin()->first;
    }

    const_pop_ptr_t GetFirstPop() const {
      if (pos_map.size() == 0) return nullptr;
      else return pos_map.begin()->first;
    }

    const_pop_ptr_t ConstGetFirstPop() const { return GetFirstPop(); }

    emp::DataLayout & GetDataLayout() {
      emp_assert(GetFirstPop());
      return GetFirstPop()->GetDataLayout();
    }
    const emp::DataLayout & GetDataLayout() const {
      emp_assert(GetFirstPop());
      return GetFirstPop()->GetDataLayout();
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
    ConstCollectionIterator cbegin() const { return ConstCollectionIterator(this); }
    ConstCollectionIterator cend() const { return ConstCollectionIterator(this, nullptr); }

    /// Remove all entries from a collection.
    Collection & Clear() { pos_map.clear(); return *this; }

    /// Add a Population to this collection.
    template <typename... Ts>
    Collection & Insert(Population & pop, Ts &&... extras) {
      PopInfo & pop_info = pos_map[&pop];
      pop_info.full_pop = true;
      pop_info.is_mutable = true;
      return Insert( std::forward<Ts>(extras)... );  // Insert anything else provided.
    }

    /// Add a const Population to this collection.
    template <typename... Ts>
    Collection & Insert(const Population & pop, Ts &&... extras) {
      emp::Ptr<const Population> pop_ptr = &pop;
      pos_map[pop_ptr.ConstCast<Population>()].full_pop = true;
      return Insert( std::forward<Ts>(extras)... );  // Insert anything else provided.
    }

    /// Add an organism (by position!)
    template <typename... Ts>
    Collection & Insert(OrgPosition pos, Ts &&... extras) {
      PopInfo & pop_info = pos_map[pos.PopPtr()];
      pop_info.InsertPos(pos.Pos());
      pop_info.is_mutable = true;
      return Insert( std::forward<Ts>(extras)... );  // Insert anything else provided.
    }

    /// Add a const organism (by position!)
    template <typename... Ts>
    Collection & Insert(ConstOrgPosition pos, Ts &&... extras) {
      PopInfo & pop_info = pos_map[pos.PopPtr()];
      pop_info.InsertPos(pos.Pos());
      return Insert( std::forward<Ts>(extras)... );  // Insert anything else provided.
    }

    template <typename... Ts>
    Collection & Insert(PopIterator pi, Ts &&... extras) { 
      return Insert( pi.AsPosition(), std::forward<Ts>(extras)... );
    }

    template <typename... Ts>
    Collection & Insert(ConstPopIterator pi, Ts &&... extras) { 
      return Insert( pi.AsPosition(), std::forward<Ts>(extras)... );
    }

    /// Add a whole other collection.
    template <typename... Ts>
    Collection & Insert(const Collection & in_collection, Ts &&... extras) {
      for (auto & [pop_ptr, in_pop_info] : in_collection.pos_map) {
        PopInfo & pop_info = pos_map[pop_ptr];

        // If the incoming collection has mutable access to a population, this one should too.
        if (in_pop_info.is_mutable) pop_info.is_mutable = true;

        // If we already have a full population, we are done!.
        if (pop_info.full_pop) continue;

        // If we're adding a full population, do so.
        if (in_pop_info.full_pop) { pop_info.full_pop = true; continue; }

        // Otherwise add just the entries we need to.
        emp::BitVector & pos_set = pop_info.pos_set;
        emp::BitVector in_pos_set = in_pop_info.pos_set;

        // Make sure both position sets are the size of the larger one.
        if (in_pos_set.GetSize() < pos_set.GetSize()) {
          in_pos_set.Resize(pos_set.GetSize());
        }
        else if (pos_set.GetSize() < in_pos_set.GetSize()) {
          pos_set.Resize(in_pos_set.GetSize());
        }

        // Use 'OR' to find the union of the sets.
        pos_set |= in_pos_set;
      }

      return Insert( std::forward<Ts>(extras)... );  // Insert anything else provided.
    }

    /// Base case... nothing left to insert.
    Collection & Insert() { return *this; }

    /// Set this collection to be exactly the provided items.
    template <typename... Ts>
    Collection & Set(Ts &&...args) {
      Clear();
      return Insert( std::forward<Ts>(args)... );
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
        for (int pos = pos_set.FindOne(); pos != -1; pos = pos_set.FindOne(pos+1)) {
          if (!pop_ptr->IsOccupied((size_t) pos)) pos_set.Set(pos,false);
        }
      }

      return *this;
    }

    /// Produce a new collection limited to living organisms.
    Collection GetAlive() const {
      Collection out(*this);
      out.RemoveEmpty();
      return out;
    }

    /// Merge this collection with another collection.
    Collection & operator|= (const Collection & in) { return Insert(in); }
    
    /// Shortcut to insert anything into this collection.
    template <typename T> Collection & operator+= (const T & in) { return Insert(in); }

    /// Reduce to the intersection with another collection.
    Collection & operator&= (const Collection & in_collection) {
      auto cur_it = pos_map.begin();
      auto in_it = in_collection.pos_map.begin();

      // Step through both iterators dealing appropriately with populations.
      while (cur_it != pos_map.end() && in_it != in_collection.pos_map.end()) {
        // If the 'in' iterator is smaller, ignore this population and move on.
        if (in_it->first < cur_it->first) { in_it++; continue; }
        
        // If the current iterator is smaller, delete the current population (not in intersection)
        if (cur_it->first < in_it->first) { cur_it = pos_map.erase(cur_it); continue; }

        // Otherwise populations must be the same!  If 'in' pop is full, keep this one as is!
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

    static std::string EMPGetTypeName() { return "mabe::Collection"; }
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

  /// Constructor where you can optionally supply collection pointer and position.
  template <typename DERIVED_T, typename ORG_T, typename COLLECTION_T>
  CollectionIterator_Interface<DERIVED_T, ORG_T, COLLECTION_T>
    ::CollectionIterator_Interface(emp::Ptr<COLLECTION_T> _col, size_t _pos)
    : base_t(_col->GetFirstPop(), _pos), collection_ptr(_col)
  {
    // Make sure that this iterator is actually valid.  If not, move to next position.
    if (base_t::IsValid() == false) IncPosition();
  }

  /// Constructor where you can optionally supply collection pointer and position.
  template <typename DERIVED_T, typename ORG_T, typename COLLECTION_T>
  CollectionIterator_Interface<DERIVED_T, ORG_T, COLLECTION_T>
    ::CollectionIterator_Interface(emp::Ptr<COLLECTION_T> _col, emp::Ptr<Population> pop, size_t _pos)
    : base_t(pop, _pos), collection_ptr(_col)
  {
  }

  /// Constructor where you supply a collection reference and optional position.
  template <typename DERIVED_T, typename ORG_T, typename COLLECTION_T>
  CollectionIterator_Interface<DERIVED_T, ORG_T, COLLECTION_T>
    ::CollectionIterator_Interface(COLLECTION_T & _col, size_t _pos)
    : base_t(_col.GetFirstPop(), _pos), collection_ptr(&_col)
  {
    // Make sure that this iterator is actually valid.  If not, move to next position.
    if (base_t::IsValid() == false) IncPosition();
  }

}

#endif
