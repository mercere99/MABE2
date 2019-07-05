/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Population.h
 *  @brief Container for a group of arbitrary MABE organisms.
 *
 *  Organisms in MABE are stored in Population objects.
 *  A single position in a Population object is described by a Population::Position.
 */

#ifndef MABE_POPULATION_H
#define MABE_POPULATION_H

#include <string>

#include "base/Ptr.h"
#include "base/vector.h"

#include "Organism.h"

namespace mabe {

  /// An EmptyOrganism is used as a placeholder in an empty cell in a population.
  class EmptyOrganism : public Organism {
    emp::Ptr<Organism> Clone() override { emp_assert(false, "Do not clone EmptyOrganism"); return nullptr; }
    std::string ToString() override { return "[empty]"; }
    int Mutate(emp::Random &) override { emp_assert(false, "EmptyOrganism cannot Mutate()"); return -1; }
    int Randomize(emp::Random &) override { emp_assert(false, "EmptyOrganism cannot Randomize()"); return -1; }
    void GenerateOutput(const std::string &, size_t) override { emp_assert(false, "EmptyOrganism cannot GenerateOutput()"); }
    emp::TypeID GetOutputType(size_t=0) override { return emp::TypeID(); }
    bool IsEmpty() const noexcept override { return true; }
  };

  class Population {
  private:
    std::string name="";                   ///< Unique name for this population.
    size_t world_id = (size_t) -1;         ///< Position in world of this population.
    emp::vector<emp::Ptr<Organism>> orgs;  ///< Info on all organisms in this population.
    bool skip_empty = false;               ///< When iterating, should we skip over empty cells?

    EmptyOrganism empty_org;               ///< Organism to fill in empty cells (does have data map!)

  public:
    class Iterator {
     ///  @todo Add a const interator, and probably a reverse iterator.
     ///  @todo Fix operator-- which can go off of the beginning of the world.
    private:
      emp::Ptr<Population> pop_ptr;
      size_t pos;
      bool skip_empty;

    public:
      Iterator(emp::Ptr<Population> _pop, size_t _pos=0, bool _skip=true)
        : pop_ptr(_pop), pos(_pos), skip_empty(_skip) { if (skip_empty) ToOccupied(); }
      Iterator(const Iterator &) = default;
      Iterator & operator=(const Iterator &) = default;

      // Shortcuts to retrieve information from the POPULATION.
      const std::string & PopName() const { emp_assert(pop_ptr); return pop_ptr->name; }
      int PopID() const { emp_assert(pop_ptr); return pop_ptr->id; }
      size_t PopSize() const { emp_assert(pop_ptr); return pop_ptr->orgs.size(); }
      emp::Ptr<Organism> OrgPtr() { emp_assert(pop_ptr); return pop_ptr->orgs[pos]; }
      emp::Ptr<const Organism> OrgPtr() const { emp_assert(pop_ptr); return pop_ptr->orgs[pos]; }

      // Other information about this iterator.
      size_t Pos() const noexcept { return pos; };
      bool SkipEmpty() const noexcept { return skip_empty; };

      void Pos(size_t in) { pos = in; }
      void SkipEmpty(bool in) { skip_empty = in; if (skip_empty) ToOccupied(); }

      /// Is the pointed-to cell occupied?
      bool IsValid() const { return pos < PopSize(); }
      bool IsEmpty() const { return IsValid() && OrgPtr()->IsEmpty(); }
      bool IsOccupied() const { return IsValid() && !OrgPtr()->IsEmpty(); }

      /// If on empty cell, advance iterator to next non-null position (or the end)
      void ToOccupied() { while (pos < PopSize() && OrgPtr()->IsEmpty()) ++pos; }

      /// Move to the first empty cell after 'start'.
      void ToOccupied(size_t start) { pos = start; ToOccupied(); }

      /// Advance iterator to the next non-empty cell in the world.
      Iterator & operator++() {
        ++pos;
        if (skip_empty) ToOccupied();
        return *this;
      }

      /// Backup iterator to the previos non-empty cell in the world.
      Iterator & operator--() {
        --pos;
        if (skip_empty) {
          while (pos < PopSize() && OrgPtr()->IsEmpty()) --pos;
        }
        return *this;
      }

      /// Iterator comparisons (iterators from different populations have no ordinal relationship).
      bool operator==(const Iterator& in) const { return pop_ptr == in.pop_ptr && pos == in.pos; }
      bool operator!=(const Iterator& in) const { return pop_ptr != in.pop_ptr || pos != in.pos; }
      bool operator< (const Iterator& in) const { return pop_ptr == in.pop_ptr && pos <  in.pos; }
      bool operator<=(const Iterator& in) const { return pop_ptr == in.pop_ptr && pos <= in.pos; }
      bool operator> (const Iterator& in) const { return pop_ptr == in.pop_ptr && pos >  in.pos; }
      bool operator>=(const Iterator& in) const { return pop_ptr == in.pop_ptr && pos >= in.pos; }

      /// Return a reference to the organism pointed to by this iterator; may advance iterator.
      Organism & operator*() {
        if (skip_empty) ToOccupied();  // If we only want occupied cells, make sure we're on one.
        emp_assert(IsValid());      // Make sure we're not outside of the vector.
        return *(OrgPtr());
      }

      /// Return a const reference to the organism pointed to by this iterator.
      /// Note that since this version is const, it will NOT advance the iterator.
      const Organism & operator*() const { emp_assert(IsValid()); return *(OrgPtr()); }

      /// Is this iterator pointing to a valid cell in the world?
      operator bool() const { return pos < PopSize() && IsOccupied(); }

      /// Return an iterator pointing to the first occupied cell in the world.
      Iterator begin() { return Iterator(pop_ptr, 0, skip_empty); }

      /// Return a const iterator pointing to the first occupied cell in the world.
      const Iterator begin() const { return Iterator(pop_ptr, 0, skip_empty); }

      /// Return an iterator pointing to just past the end of the world.
      Iterator end() { return Iterator(pop_ptr, PopSize(), skip_empty); }

      /// Return a const iterator pointing to just past the end of the world.
      const Iterator end() const { return Iterator(pop_ptr, PopSize(), skip_empty); }
    };

    class ConstIterator {
    private:
      emp::Ptr<const Population> pop_ptr;
      size_t pos;
      bool skip_empty;

    public:
      ConstIterator(emp::Ptr<const Population> _pop, size_t _pos=0, bool _skip=true)
        : pop_ptr(_pop), pos(_pos), skip_empty(_skip) { if (skip_empty) ToOccupied(); }
      ConstIterator(const ConstIterator &) = default;
      ConstIterator & operator=(const ConstIterator &) = default;

      // Shortcuts to retrieve information from the POPULATION.
      const std::string & PopName() const { emp_assert(pop_ptr); return pop_ptr->name; }
      int PopID() const { emp_assert(pop_ptr); return pop_ptr->id; }
      size_t PopSize() const { emp_assert(pop_ptr); return pop_ptr->orgs.size(); }
      emp::Ptr<const Organism> OrgPtr() const { emp_assert(pop_ptr); return pop_ptr->orgs[pos]; }

      // Other information about this Constiterator.
      size_t Pos() const noexcept { return pos; };
      bool SkipEmpty() const noexcept { return skip_empty; };

      void Pos(size_t in) { pos = in; }
      void SkipEmpty(bool in) { skip_empty = in; if (skip_empty) ToOccupied(); }

      /// Is the pointed-to cell occupied?
      bool IsValid() const { return pos < PopSize(); }
      bool IsEmpty() const { return IsValid() && OrgPtr()->IsEmpty(); }
      bool IsOccupied() const { return IsValid() && !OrgPtr()->IsEmpty(); }

      /// If on empty cell, advance Constiterator to next non-null position (or the end)
      void ToOccupied() { while (pos < PopSize() && OrgPtr()->IsEmpty()) ++pos; }

      /// Move to the first empty cell after 'start'.
      void ToOccupied(size_t start) { pos = start; ToOccupied(); }

      /// Advance Constiterator to the next non-empty cell in the world.
      ConstIterator & operator++() {
        ++pos;
        if (skip_empty) ToOccupied();
        return *this;
      }

      /// Backup Constiterator to the previos non-empty cell in the world.
      ConstIterator & operator--() {
        --pos;
        if (skip_empty) {
          while (pos < PopSize() && OrgPtr()->IsEmpty()) --pos;
        }
        return *this;
      }

      /// ConstIterator comparisons (Constiterators from different populations have no ordinal relationship).
      bool operator==(const ConstIterator& in) const { return pop_ptr == in.pop_ptr && pos == in.pos; }
      bool operator!=(const ConstIterator& in) const { return pop_ptr != in.pop_ptr || pos != in.pos; }
      bool operator< (const ConstIterator& in) const { return pop_ptr == in.pop_ptr && pos <  in.pos; }
      bool operator<=(const ConstIterator& in) const { return pop_ptr == in.pop_ptr && pos <= in.pos; }
      bool operator> (const ConstIterator& in) const { return pop_ptr == in.pop_ptr && pos >  in.pos; }
      bool operator>=(const ConstIterator& in) const { return pop_ptr == in.pop_ptr && pos >= in.pos; }

      /// Return a reference to the organism pointed to by this iterator; may advance iterator.
      const Organism & operator*() {
        if (skip_empty) ToOccupied();  // If we only want occupied cells, make sure we're on one.
        emp_assert(IsValid());      // Make sure we're not outside of the vector.
        return *(OrgPtr());
      }

      /// Return a const reference to the organism pointed to by this Constiterator.
      /// Note that since this version is const, it will NOT advance the Constiterator.
      const Organism & operator*() const { emp_assert(IsValid()); return *(OrgPtr()); }

      /// Is this Constiterator pointing to a valid cell in the world?
      operator bool() const { return pos < PopSize() && IsOccupied(); }

      /// Return an Constiterator pointing to the first occupied cell in the world.
      ConstIterator begin() { return ConstIterator(pop_ptr, 0, skip_empty); }

      /// Return a const Constiterator pointing to the first occupied cell in the world.
      const ConstIterator begin() const { return ConstIterator(pop_ptr, 0, skip_empty); }

      /// Return an Constiterator pointing to just past the end of the world.
      ConstIterator end() { return ConstIterator(pop_ptr, PopSize(), skip_empty); }

      /// Return a const Constiterator pointing to just past the end of the world.
      const ConstIterator end() const { return ConstIterator(pop_ptr, PopSize(), skip_empty); }
    };
    
  private:
    /// A placement function takes a position in THIS population and returns an Interator
    /// indicating where the organism at that position should place its offspring.
    using placement_fun_t = std::function< Iterator(size_t) >;
    placement_fun_t placement_fun;

  public:
    Population(const std::string & in_name, size_t in_id, size_t pop_size) : name(in_name), world_id(in_id) {
      orgs.resize(pop_size, &empty_org);
    }
    Population(const Population & in_pop) : name(in_pop.name + "_copy"), orgs(in_pop.orgs.size()) {
      for (size_t i = 0; i < orgs.size(); i++) {
        if (in_pop.orgs[i]->IsEmpty()) {       // Make sure we always use local empty organism.
          orgs[i] = &empty_org;
        } else {                              // Otherwise clone the organism.
          orgs[i] = in_pop.orgs[i]->Clone();
        }
      }
    }
    Population(Population &&) = default;

    ~Population() { for (auto x : orgs) x.Delete(); }

    const std::string & GetName() const noexcept { return name; }
    int GetWorldID() const noexcept { return world_id; }
    size_t GetSize() const noexcept { return orgs.size(); }
    bool GetSkipEmpty() const noexcept { return skip_empty; }

    void SetWorldID(int in_id) noexcept { world_id = in_id; }
    void SetSkipEmpty(bool in=true) { skip_empty = in; }

    Organism & operator[](size_t org_id) { return *(orgs[org_id]); }
    const Organism & operator[](size_t org_id) const { return *(orgs[org_id]); }

    /// Return an iterator pointing to the first occupied cell in the world.
    Iterator begin() { return Iterator(this, 0, skip_empty); }
    Iterator begin(bool skip_empty) { return Iterator(this, 0, skip_empty); }

    /// Return a const iterator pointing to the first occupied cell in the world.
    ConstIterator begin() const { return ConstIterator(this, 0, skip_empty); }
    ConstIterator begin(bool skip_empty) const { return ConstIterator(this, 0, skip_empty); }

    /// Return an iterator pointing to just past the end of the world.
    Iterator end() { return Iterator(this, GetSize(), skip_empty); }
    Iterator end(bool skip_empty) { return Iterator(this, GetSize(), skip_empty); }

    /// Return a const iterator pointing to just past the end of the world.
    ConstIterator end() const { return ConstIterator(this, GetSize(), skip_empty); }
    ConstIterator end(bool skip_empty) const { return ConstIterator(this, GetSize(), skip_empty); }

    Iterator IteratorAt(size_t pos) { return Iterator(this, pos); }
    ConstIterator ConstIteratorAt(size_t pos) const { return ConstIterator(this, pos); }

    // All insertions of organisms should come through this function.
    void AddOrgAt(emp::Ptr<Organism> org_ptr, size_t pos, size_t ppos) {
      // @CAO: TRIGGER BEFORE PLACEMENT SIGNAL! Include both new organism and parent, if available.
      RemoveOrgAt(pos);     // Clear out any organism already in this position.
      orgs[pos] = org_ptr;  // Put the new organism in place.
      // @CAO: Indicate +1 organism in the population.
      // @CAO: TRIGGER ON PLACEMENT SIGNAL!
    }

    /// All removal of organisms should come through this function.
    void RemoveOrgAt(size_t pos) {
      emp_assert(pos < orgs.size());
      if (orgs[pos].IsEmpty()) return; // Nothing to remove!

      // @CAO: TRIGGER BEFORE DEATH SIGNAL!
      orgs[pos].Delete();
      orgs[pos] = &empty_org;
      // @CAO Indicate -1 organism in the population.
    }

    void Resize(size_t new_size) {
      // Clean up any organisms that may be getting deleted.
      for (size_t pos = new_size; pos < orgs.size(); pos++) {
        RemoveOrgAt(pos);
      }

      // Resize the population, adding in empty cells to any new spaces.
      orgs.resize(new_size, &empty_org);
    }

    /// Add an empty position to the end of the population (and return an iterator to it)
    Iterator PushEmpty() {
      size_t pos = orgs.size();
      orgs.resize(orgs.size()+1, &empty_org);
      return Iterator(this, pos);
    }

    /// Set the placement function to put offspring at the end of a specified population.
    /// Organism replication and placement.
    void SetGrowthPlacement(Population & pop) {
      placement_fun = [&pop](size_t id){ return pop.PushEmpty() };
    }

    /// If we don't specific a population to place offspring in, assume they go in the current one.
    void SetGrowthPlacement() { SetGrowthPlacement(*this); }

    void Replicate(size_t org_id, size_t copy_count) {
      emp_assert(placement_fun);
      for (size_t copy_id = 0; copy_id < copy_count; copy_id++) {
        Iterator target = placement_fun(org_id);
      }
    }
  };

}

#endif
