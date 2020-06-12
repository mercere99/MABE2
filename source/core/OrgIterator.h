/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  OrgIterator.h
 *  @brief Tools to step through a group of arbitrary MABE organisms.
 *
 *  Organisms in MABE are stored in indexed collections (typically Population objects).
 *  This class allows you to refer to the position of an organism and step through sets of organisms.
 * 
 *  An OrgIterator_Interface sets up all of the virutal functions in all iterators.
 *
 *  @todo Add a reverse iterator.
 *  @todo Fix operator-- which can go off of the beginning of the world.
 */

#ifndef MABE_ORG_ITERATOR_H
#define MABE_ORG_ITERATOR_H

#include <string>

#include "base/Ptr.h"
#include "base/vector.h"

#include "../config/ConfigType.h"

#include "Organism.h"

namespace mabe {

  class Population;

  template <typename DERIVED_T, typename CONTAINER_T=Population, typename INDEX_T=size_t>
  class OrgIterator_Interface {
    friend class MABEBase;
  protected:
    emp::Ptr<CONTAINER_T> pop_ptr;
    INDEX_T pos;

    using this_t = OrgIterator_Interface<DERIVED_T, CONTAINER_T, INDEX_T>;

    // Helper functions to be overloaded in derived classes.
    virtual void ShiftPosition(int=1) = 0;
    virtual void ToBegin() = 0;
    virtual void ToEnd() = 0;
    virtual void MakeValid() = 0;

  public:
    /// Constructor where you can optionally supply population pointer and position.
    OrgIterator_Interface(emp::Ptr<CONTAINER_T> _pop=nullptr, INDEX_T _pos=0)
      : pop_ptr(_pop), pos(_pos) { }

    /// Supply CONTAINER_T by reference instead of pointer.
    OrgIterator_Interface(CONTAINER_T & pop, INDEX_T _pos=0) : OrgIterator_Interface(&pop, _pos) {}

    /// Copy constructor
    OrgIterator_Interface(const OrgIterator_Interface &) = default;

    /// Destructor
    virtual ~OrgIterator_Interface() { }

    /// Copy operator
    this_t & operator=(const OrgIterator_Interface & in) = default;

    // Shortcut definitions to retrieve information from the POPULATION.
    const std::string & PopName() const { emp_assert(pop_ptr); return pop_ptr->GetName(); }
    int PopID() const { emp_assert(pop_ptr); return pop_ptr->GetID(); }
    size_t PopSize() const { emp_assert(pop_ptr); return pop_ptr->GetSize(); }
    emp::Ptr<Organism> OrgPtr() { emp_assert(pop_ptr); return &(*pop_ptr)[pos]; }
    emp::Ptr<const Organism> OrgPtr() const { emp_assert(pop_ptr); return &(*pop_ptr)[pos]; }

    // Information direct from this OrgPosition.
    INDEX_T Pos() const noexcept { return pos; };
    emp::Ptr<CONTAINER_T> PopPtr() noexcept { return pop_ptr; }

    std::string ToString() const {
      return emp::to_string("{pop_ptr=", pop_ptr, ";pos=", pos, "}");
    }

    DERIVED_T & SetPos(INDEX_T in) { pos = in; return (DERIVED_T &) *this; }

    /// Is this iterator currently in a legal state?
    bool IsValid() const { return !pop_ptr.IsNull() && pos < PopSize(); }

    /// Is the pointed-to cell occupied?
    bool IsEmpty() const { return IsValid() && OrgPtr()->IsEmpty(); }
    bool IsOccupied() const { return IsValid() && !OrgPtr()->IsEmpty(); }

    /// Advance iterator to the next non-empty cell in the world.
    DERIVED_T & operator++() { ShiftPosition(1); return (DERIVED_T &) *this; }

    /// Postfix++: advance iterator to the next non-empty cell in the world.
    DERIVED_T operator++(int) {
      DERIVED_T out = (DERIVED &) *this;
      ShiftPosition(1);
      return out;
    }

    /// Backup iterator to the previos non-empty cell in the world.
    DERIVED_T & operator--() {
      ShiftPosition(-1);
      return (DERIVED_T &) *this;
    }

    /// Postfix--: Backup iterator to the previos non-empty cell in the world.
    DERIVED_T operator--(int) {
      DERIVED_T out = (DERIVED_T &) *this;
      ShiftPosition(-1);;
      return out;
    }

    // Basic math operations...
    DERIVED_T operator+(int x) {
      DERIVED_T out = (DERIVED_T &) *this;
      out.ShiftPosition(x);
      return out;
    }

    DERIVED_T operator-(int x) {
      DERIVED_T out = (DERIVED_T &) *this;
      out.ShiftPosition(-x);
      return out;
    }

    // Compound math operations...
    DERIVED_T & operator+=(int x) {
      ShiftPosition(x);
      return (DERIVED_T &) *this;
    }

    DERIVED_T & operator-=(int x) {
      ShiftPosition(-x);
      return (DERIVED_T &) *this;
    }

    /// OrgPosition comparisons (OrgPositions from different populations have no ordinal relationship).
    bool operator==(const OrgPosition& in) const { return pop_ptr == in.pop_ptr && pos == in.pos; }
    bool operator!=(const OrgPosition& in) const { return pop_ptr != in.pop_ptr || pos != in.pos; }
    bool operator< (const OrgPosition& in) const { return pop_ptr == in.pop_ptr && pos <  in.pos; }
    bool operator<=(const OrgPosition& in) const { return pop_ptr == in.pop_ptr && pos <= in.pos; }
    bool operator> (const OrgPosition& in) const { return pop_ptr == in.pop_ptr && pos >  in.pos; }
    bool operator>=(const OrgPosition& in) const { return pop_ptr == in.pop_ptr && pos >= in.pos; }

    /// Return a reference to the organism pointed to by this OrgPosition; may advance OrgPosition.
    Organism & operator*() {
      MakeValid();              // If the population has changed, adjust appropriately.
      emp_assert(IsValid());    // Make sure we're not outside of the vector.
      return *(OrgPtr());
    }

    /// Return a const reference to the organism pointed to by this OrgPosition.
    /// Note that since this version is const, it will NOT adjust the OrgPosition.
    const Organism & operator*() const { emp_assert(IsValid()); return *(OrgPtr()); }

    /// Allow OrgPosition to be used as a pointer.
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

    /// Is this OrgPosition pointing to a valid cell in the world?
    operator bool() const { return pos < PopSize() && IsOccupied(); }

    /// OrgPositions can be automatically converted to a pointer to the organism they refer to.
    operator emp::Ptr<mabe::Organism>() { emp_assert(IsValid()); return OrgPtr(); }

    /// Return an iterator pointing to the first occupied cell in the world.
    DERIVED_T begin() const { DERIVED_T out( (DERIVED_T &) *this); out.ToBegin(); return out; }

    /// Return an iterator pointing to just past the end of the world.
    DERIVED_T end() const { DERIVED_T out( (DERIVED_T &) *this); out.ToEnd(); return out; }

  private:  // ---== To be used by friend class MABEBase only! ==---
    /// Insert an organism into the pointed-at position.
    void SetOrg(emp::Ptr<Organism> org_ptr) { pop_ptr->SetOrg(pos, org_ptr); }

    /// Remove the organism at the pointed-at position and return it.
    [[nodiscard]] emp::Ptr<Organism> ExtractOrg() { return pop_ptr->ExtractOrg(pos); }
  };

  class OrgPosition : public OrgIterator_Interface<OrgPosition> {
  protected:
    using base_t = OrgIterator_Interface<OrgPosition>;

    void ShiftPosition(int=1) override { emp_error("ShiftPosition(shift_size) not defined in OrgPosition."); }
    void ToBegin() override { emp_error("ToBegin() not defined in OrgPosition."); }
    void ToEnd() override { emp_error("ToEnd() not defined in OrgPosition."); }
    void MakeValid() override { }

  public:
    /// Constructor where you can optionally supply population pointer and position.
    OrgPosition(emp::Ptr<Population> _pop=nullptr, size_t _pos=0) : base_t(_pop, _pos) { ; }

    /// Supply Population by reference instead of pointer.
    OrgPosition(Population & pop, size_t _pos=0) : OrgPosition(&pop, _pos) {}

    /// Copy constructor
    OrgPosition(const OrgPosition &) = default;

    /// Copy operator
    OrgPosition & operator=(const OrgPosition & in) = default;
  };


}

#endif
