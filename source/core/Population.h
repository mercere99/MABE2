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
#include "EmptyOrganism.h"

namespace mabe {

  class Population {
    friend class MABEBase;
  private:
    std::string name="";                   ///< Unique name for this population.
    size_t pop_id = (size_t) -1;           ///< Position in world of this population.
    emp::vector<emp::Ptr<Organism>> orgs;  ///< Info on all organisms in this population.
    size_t num_orgs = 0;                   ///< How many living organisms are in this population?
    size_t max_orgs = (size_t) -1;         ///< Maximum number of orgs allowed in population.

    EmptyOrganism empty_org;               ///< Organism to fill in empty cells (does have data map!)

  public:
    class iterator {
     ///  @todo Add a reverse iterator.
     ///  @todo Fix operator-- which can go off of the beginning of the world.
     friend class MABEBase;
    private:
      emp::Ptr<Population> pop_ptr;
      size_t pos;
      bool skip_empty;

    public:
      iterator(emp::Ptr<Population> _pop=nullptr, size_t _pos=0, bool _skip=false)
        : pop_ptr(_pop), pos(_pos), skip_empty(_skip)
      {
        if (skip_empty) ToOccupied();
      }
      iterator(Population & pop, size_t _pos=0, bool _skip=false) : iterator(&pop, _pos, _skip) {}
      iterator(const iterator &) = default;
      iterator & operator=(const iterator & in) = default;

      // Shortcuts to retrieve information from the POPULATION.
      const std::string & PopName() const { emp_assert(pop_ptr); return pop_ptr->name; }
      int PopID() const { emp_assert(pop_ptr); return pop_ptr->pop_id; }
      size_t PopSize() const { emp_assert(pop_ptr); return pop_ptr->orgs.size(); }
      emp::Ptr<Organism> OrgPtr() { emp_assert(pop_ptr); return pop_ptr->orgs[pos]; }
      emp::Ptr<const Organism> OrgPtr() const { emp_assert(pop_ptr); return pop_ptr->orgs[pos]; }

      // Other information about this iterator.
      size_t Pos() const noexcept { return pos; };
      emp::Ptr<Population> PopPtr() noexcept { return pop_ptr; }
      bool SkipEmpty() const noexcept { return skip_empty; };

      std::string ToString() const {
        return emp::to_string("{pop_ptr=", pop_ptr, ";pos=", pos, ";skip_empty=", skip_empty, "}");
      }

      iterator & Pos(size_t in) { pos = in; return *this; }
      iterator & SkipEmpty(bool in) { skip_empty = in; if (skip_empty) ToOccupied(); return *this; }

      /// Is this iterator currently in a legal state?
      bool IsValid() const { return !pop_ptr.IsNull() && pos < PopSize(); }

      /// Is the pointed-to cell occupied?
      bool IsEmpty() const { return IsValid() && OrgPtr()->IsEmpty(); }
      bool IsOccupied() const { return IsValid() && !OrgPtr()->IsEmpty(); }

      /// If on empty cell, advance iterator to next non-null position (or the end)
      void ToOccupied() { while (pos < PopSize() && OrgPtr()->IsEmpty()) ++pos; }

      /// Move to the first empty cell after 'start'.
      void ToOccupied(size_t start) { pos = start; ToOccupied(); }

      /// Advance iterator to the next non-empty cell in the world.
      iterator & operator++() {
        ++pos;
        if (skip_empty) ToOccupied();
        return *this;
      }

      /// Postfix++: advance iterator to the next non-empty cell in the world.
      iterator operator++(int) {
        iterator out = *this;
        ++pos;
        if (skip_empty) ToOccupied();
        return out;
      }

      /// Backup iterator to the previos non-empty cell in the world.
      iterator & operator--() {
        --pos;
        if (skip_empty) { while (pos < PopSize() && OrgPtr()->IsEmpty()) --pos; }
        return *this;
      }

      /// Postfix--: Backup iterator to the previos non-empty cell in the world.
      iterator operator--(int) {
        iterator out = *this;
        --pos;
        if (skip_empty) { while (pos < PopSize() && OrgPtr()->IsEmpty()) --pos; }
        return out;
      }

      // Basic math operations...
      iterator operator+(size_t x) {
        emp_assert(pos + x <= PopSize(), pos, x, PopSize());
        return iterator(pop_ptr, pos+x);
      }

      iterator operator-(size_t x) {
        emp_assert(pos >= x);
        emp_assert(pos - x <= PopSize());
        return iterator(pop_ptr, pos-x);
      }

      // Compound math operations...
      iterator & operator+=(size_t x) {
        emp_assert(pos + x <= PopSize());
        pos += x;
        return *this;
      }

      iterator & operator-=(size_t x) {
        emp_assert(pos - x <= PopSize());
        pos -= x;
        return *this;
      }

      /// iterator comparisons (iterators from different populations have no ordinal relationship).
      bool operator==(const iterator& in) const { return pop_ptr == in.pop_ptr && pos == in.pos; }
      bool operator!=(const iterator& in) const { return pop_ptr != in.pop_ptr || pos != in.pos; }
      bool operator< (const iterator& in) const { return pop_ptr == in.pop_ptr && pos <  in.pos; }
      bool operator<=(const iterator& in) const { return pop_ptr == in.pop_ptr && pos <= in.pos; }
      bool operator> (const iterator& in) const { return pop_ptr == in.pop_ptr && pos >  in.pos; }
      bool operator>=(const iterator& in) const { return pop_ptr == in.pop_ptr && pos >= in.pos; }

      /// Return a reference to the organism pointed to by this iterator; may advance iterator.
      Organism & operator*() {
        if (skip_empty) ToOccupied();  // If we only want occupied cells, make sure we're on one.
        emp_assert(IsValid());      // Make sure we're not outside of the vector.
        return *(OrgPtr());
      }

      /// Return a const reference to the organism pointed to by this iterator.
      /// Note that since this version is const, it will NOT advance the iterator.
      const Organism & operator*() const { emp_assert(IsValid()); return *(OrgPtr()); }

      /// Allow iterator to be used as a pointer.
      emp::Ptr<mabe::Organism> operator->() {
        // Make sure a pointer is active before we follow it.
        emp_assert(IsValid());
        return OrgPtr();
      }

      /// Follow a pointer to a const target.
      emp::Ptr<const mabe::Organism> operator->() const {
        // Make sure a pointer is active before we follow it.
        emp_assert(IsValid());
        return OrgPtr();
      }

      /// Is this iterator pointing to a valid cell in the world?
      operator bool() const { return pos < PopSize() && IsOccupied(); }

      /// Iterators can be automatically converted to a pointer to the organism they refer to.
      operator emp::Ptr<mabe::Organism>() { emp_assert(IsValid()); return OrgPtr(); }

      /// Return an iterator pointing to the first occupied cell in the world.
      iterator begin() { return iterator(pop_ptr, 0, skip_empty); }

      /// Return a const iterator pointing to the first occupied cell in the world.
      const iterator begin() const { return iterator(pop_ptr, 0, skip_empty); }

      /// Return an iterator pointing to just past the end of the world.
      iterator end() { return iterator(pop_ptr, PopSize(), skip_empty); }

      /// Return a const iterator pointing to just past the end of the world.
      const iterator end() const { return iterator(pop_ptr, PopSize(), skip_empty); }

    private:  // ---== To be used by friend class MABEBase only! ==---
      /// Insert an organism into the pointed-at position.
      void SetOrg(emp::Ptr<Organism> org_ptr) { pop_ptr->SetOrg(pos, org_ptr); }
    
      /// Remove the organism at the pointed-at position and return it.
      [[nodiscard]] emp::Ptr<Organism> ExtractOrg() { return pop_ptr->ExtractOrg(pos); }

    };

    class const_iterator {
    private:
      emp::Ptr<const Population> pop_ptr;
      size_t pos;
      bool skip_empty;

    public:
      const_iterator(emp::Ptr<const Population> _pop, size_t _pos=0, bool _skip=true)
        : pop_ptr(_pop), pos(_pos), skip_empty(_skip) { if (skip_empty) ToOccupied(); }
      const_iterator(const const_iterator &) = default;
      const_iterator & operator=(const const_iterator &) = default;

      // Shortcuts to retrieve information from the POPULATION.
      const std::string & PopName() const { emp_assert(pop_ptr); return pop_ptr->name; }
      int PopID() const { emp_assert(pop_ptr); return pop_ptr->pop_id; }
      size_t PopSize() const { emp_assert(pop_ptr); return pop_ptr->orgs.size(); }
      emp::Ptr<const Organism> OrgPtr() const { emp_assert(pop_ptr); return pop_ptr->orgs[pos]; }

      // Other information about this Constiterator.
      size_t Pos() const noexcept { return pos; };
      bool SkipEmpty() const noexcept { return skip_empty; };

      const_iterator & Pos(size_t in) { pos = in; return *this; }
      const_iterator & SkipEmpty(bool in) { skip_empty = in; if (skip_empty) ToOccupied(); return *this; }

      /// Is the pointed-to cell occupied?
      bool IsValid() const { return pos < PopSize(); }
      bool IsEmpty() const { return IsValid() && OrgPtr()->IsEmpty(); }
      bool IsOccupied() const { return IsValid() && !OrgPtr()->IsEmpty(); }

      /// If on empty cell, advance Constiterator to next non-null position (or the end)
      const_iterator & ToOccupied() { while (pos < PopSize() && OrgPtr()->IsEmpty()) ++pos; return *this; }

      /// Move to the first empty cell after 'start'.
      const_iterator & ToOccupied(size_t start) { pos = start; ToOccupied(); return *this; }

      /// Advance Constiterator to the next non-empty cell in the world.
      const_iterator & operator++() {
        ++pos;
        if (skip_empty) ToOccupied();
        return *this;
      }

      /// Postfix++: advance iterator to the next non-empty cell in the world.
      const_iterator operator++(int) {
        const_iterator out = *this;
        ++pos;
        if (skip_empty) ToOccupied();
        return out;
      }

      /// Backup Constiterator to the previos non-empty cell in the world.
      const_iterator & operator--() {
        --pos;
        if (skip_empty) while (pos < PopSize() && OrgPtr()->IsEmpty()) --pos;
        return *this;
      }


      /// Postfix--: Backup iterator to the previos non-empty cell in the world.
      const_iterator operator--(int) {
        const_iterator out = *this;
        --pos;
        if (skip_empty) { while (pos < PopSize() && OrgPtr()->IsEmpty()) --pos; }
        return out;
      }

      /// const_iterator comparisons (Constiterators from different populations have no ordinal relationship).
      bool operator==(const const_iterator& in) const { return pop_ptr == in.pop_ptr && pos == in.pos; }
      bool operator!=(const const_iterator& in) const { return pop_ptr != in.pop_ptr || pos != in.pos; }
      bool operator< (const const_iterator& in) const { return pop_ptr == in.pop_ptr && pos <  in.pos; }
      bool operator<=(const const_iterator& in) const { return pop_ptr == in.pop_ptr && pos <= in.pos; }
      bool operator> (const const_iterator& in) const { return pop_ptr == in.pop_ptr && pos >  in.pos; }
      bool operator>=(const const_iterator& in) const { return pop_ptr == in.pop_ptr && pos >= in.pos; }

      /// Return a reference to the organism pointed to by this iterator; may advance iterator.
      const Organism & operator*() {
        if (skip_empty) ToOccupied();  // If we only want occupied cells, make sure we're on one.
        emp_assert(IsValid());      // Make sure we're not outside of the vector.
        return *(OrgPtr());
      }

      /// Follow a pointer to a const target.
      emp::Ptr<const mabe::Organism> operator->() const {
        // Make sure a pointer is active before we follow it.
        emp_assert(IsValid());
        return OrgPtr();
      }

      /// Return a const reference to the organism pointed to by this Constiterator.
      /// Note that since this version is const, it will NOT advance the Constiterator.
      const Organism & operator*() const { emp_assert(IsValid()); return *(OrgPtr()); }

      /// Is this Constiterator pointing to a valid cell in the world?
      operator bool() const { return pos < PopSize() && IsOccupied(); }

      /// Return an Constiterator pointing to the first occupied cell in the world.
      const_iterator begin() { return const_iterator(pop_ptr, 0, skip_empty); }

      /// Return a const Constiterator pointing to the first occupied cell in the world.
      const const_iterator begin() const { return const_iterator(pop_ptr, 0, skip_empty); }

      /// Return an Constiterator pointing to just past the end of the world.
      const_iterator end() { return const_iterator(pop_ptr, PopSize(), skip_empty); }

      /// Return a const Constiterator pointing to just past the end of the world.
      const const_iterator end() const { return const_iterator(pop_ptr, PopSize(), skip_empty); }
    };
    
    /// Population wrapper to limit to just living organisms.
    class AlivePop {
    private:
      Population & pop;
    public:
      AlivePop(Population & _pop) : pop(_pop) { ; }
      iterator begin() { return pop.begin_alive(); }
      iterator end() { return pop.end_alive(); }
    };

  public:
    Population() { emp_assert(false, "Do not use default constructor on Population!"); }
    Population(const std::string & in_name, size_t in_id, size_t pop_size=0)
      : name(in_name), pop_id(in_id)
    {
      orgs.resize(pop_size, &empty_org);
    }
    Population(const Population & in_pop)
      : name(in_pop.name), pop_id(in_pop.pop_id), orgs(in_pop.orgs.size())
      , num_orgs(in_pop.num_orgs), max_orgs(in_pop.max_orgs)
    {
      emp_assert(in_pop.OK());
      for (size_t i = 0; i < orgs.size(); i++) {
        if (in_pop.orgs[i]->IsEmpty()) {       // Make sure we always use local empty organism.
          orgs[i] = &empty_org;
        } else {                              // Otherwise clone the organism.
          orgs[i] = in_pop.orgs[i]->Clone();
        }
      }
      emp_assert(OK());
    }
    Population(Population &&) = default;
    Population & operator=(Population &&) = default;

    ~Population() { for (auto x : orgs) if (!x->IsEmpty()) x.Delete(); }

    const std::string & GetName() const noexcept { return name; }
    int GetWorldID() const noexcept { return pop_id; }
    size_t GetSize() const noexcept { return orgs.size(); }
    size_t GetNumOrgs() const noexcept { return num_orgs; }

    bool IsEmpty(size_t pos) const { return orgs[pos]->IsEmpty(); }
    bool IsOccupied(size_t pos) const { return !orgs[pos]->IsEmpty(); }

    void SetWorldID(int in_id) noexcept { pop_id = in_id; }

    Organism & operator[](size_t org_id) { return *(orgs[org_id]); }
    const Organism & operator[](size_t org_id) const { return *(orgs[org_id]); }

    /// Return an iterator pointing to the first occupied cell in the world.
    iterator begin() { return iterator(this, 0, false); }
    iterator begin_alive() { return iterator(this, 0, true); }

    /// Return a const iterator pointing to the first occupied cell in the world.
    const_iterator begin() const { return const_iterator(this, 0, false); }
    const_iterator begin_alive() const { return const_iterator(this, 0, true); }

    /// Return an iterator pointing to just past the end of the world.
    iterator end() { return iterator(this, GetSize(), false); }
    iterator end_alive() { return iterator(this, GetSize(), true); }

    /// Return a const iterator pointing to just past the end of the world.
    const_iterator end() const { return const_iterator(this, GetSize(), false); }
    const_iterator end_alive() const { return const_iterator(this, GetSize(), true); }

    iterator IteratorAt(size_t pos, bool skip=false) {
      return iterator(this, pos, skip);
    }
    const_iterator ConstIteratorAt(size_t pos, bool skip=false) const {
      return const_iterator(this, pos, skip);
    }

    /// Limit iterators to LIVING organisms.
    AlivePop Alive() { return AlivePop(*this); }


  private:  // ---== To be used by friend class MABEBase only! ==---

    void SetOrg(size_t pos, emp::Ptr<Organism> org_ptr) {
      emp_assert(pos < orgs.size());
      emp_assert(IsEmpty(pos));         // Must be valid and should not overwrite a living cell.
      emp_assert(!org_ptr->IsEmpty());  // Use ClearOrg if you want to empty a cell.
      orgs[pos] = org_ptr;
      num_orgs++;
    }

    /// Remove (and return) the organism at pos, but don't delete it.
    [[nodiscard]] emp::Ptr<Organism> ExtractOrg(size_t pos) {
      emp_assert(pos < orgs.size());
      emp::Ptr<Organism> out_org = orgs[pos];
      orgs[pos] = &empty_org;
      if (!out_org->IsEmpty()) num_orgs--;
      return out_org;
    }

    /// Resize a population; should only be called from world after removed orgs are deleted.
    Population & Resize(size_t new_size) {
      emp_assert(num_orgs == 0);

      // Resize the population, adding in empty cells to any new spaces.
      orgs.resize(new_size, &empty_org);

      return *this;
    }

    /// Add an empty position to the end of the population (and return an iterator to it)
    iterator PushEmpty() {
      size_t pos = orgs.size();
      orgs.resize(orgs.size()+1, &empty_org);
      return iterator(this, pos);
    }

  public:
    // ------ DEBUG FUNCTIONS ------
    bool OK() const {
      if (pop_id < 0) {
        std::cout << "WARNING: Invalid Population ID (pop_id = " << pop_id << ")" << std::endl;
        return false;
      }

      if (num_orgs > orgs.size()) {
        std::cout << "ERROR: Population " << pop_id << " size is " << orgs.size()
                  << " but num_orgs = " << num_orgs << std::endl;
        return false;
      }

      size_t org_count = 0;
      for (size_t pos = 0; pos < orgs.size(); pos++) {
        // No vector positions should be NULL (though they may have an empty organism)
        if (orgs[pos].IsNull()) {
          std::cout << "ERROR: Population " << pop_id << " as position " << pos
                    << " has null pointer instead of an organism." << std::endl;
          return false;
        }

        // Double check the organism count.
        if (!orgs[pos]->IsEmpty()) org_count++;
      }

      // Make sure we counted the correct number of organims in the population.
      if (num_orgs != org_count) {
          std::cout << "ERROR: Population " << pop_id << " has num_orgs = " << num_orgs
                    << ", but audit counts " << org_count << " orgs." << std::endl;
          return false;
      }

      // @CAO: Check if num_orgs > max_orgs?

      return true;
    }
  };

  // Alias Population::iterator to OrgPosition for more intuitive use outside of Population.
  using OrgPosition = Population::iterator;
}

#endif
