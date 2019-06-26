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
    std::string name;                      ///< Unique name for this population.
    int id;                                ///< Position in world of this population.
    emp::vector<emp::Ptr<Organism>> orgs;  ///< Info on all organisms in this population.
    EmptyOrganism empty_org;               ///< Organism to fill in empty cells.

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

    Population(const std::string & in_name, int in_id=0) : name(in_name), id(in_id) { }
    Population(const Population & in_pop) : name(in_pop.name + "_copy"), orgs(in_pop.orgs.size()) {
      for (size_t i = 0; i < orgs.size(); i++) {
        orgs[i] = in_pop.orgs[i]->Clone();
      }
    }
    Population(Population &&) = default;

    ~Population() { for (auto x : orgs) x.Delete(); }

    const std::string & GetName() const noexcept { return name; }
    int GetID() const noexcept { return id; }
    size_t GetSize() const noexcept { return orgs.size(); }

    void SetID(int in_id) noexcept { id = in_id; }

    /// Return an iterator pointing to the first occupied cell in the world.
    Iterator begin(bool skip_empty=true) { return Iterator(this, 0, skip_empty); }

    /// Return a const iterator pointing to the first occupied cell in the world.
    const Iterator begin(bool skip_empty=true) const { return Iterator(this, 0, skip_empty); }

    /// Return an iterator pointing to just past the end of the world.
    Iterator end(bool skip_empty=true) { return Iterator(this, GetSize(), skip_empty); }

    /// Return a const iterator pointing to just past the end of the world.
    const Iterator end(bool skip_empty=true) const { return Iterator(this, GetSize(), skip_empty); }

  };

}

#endif
