/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2021.
 *
 *  @file  OrgIterator.hpp
 *  @brief Tools to step through a group of arbitrary MABE organisms.
 *
 *  Organisms in MABE are stored in indexed collections (typically Population objects).
 *  This class allows you to refer to the position of an organism and step through sets of organisms.
 * 
 *  An OrgIterator_Interface sets up all of the virtual functions in all iterators.
 *
 *  @todo Add a reverse iterator.
 *  @todo Fix operator-- which can go off of the beginning of the world.
 */

#ifndef MABE_ORG_ITERATOR_H
#define MABE_ORG_ITERATOR_H

#include <string>

#include "emp/base/error.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"

#include "Organism.hpp"

namespace mabe {

  /// Base class for all organsim containers, including population.  
  struct OrgContainer : public EmplodeType {
    virtual ~OrgContainer() { }

    virtual std::string GetName() const { return ""; }
    virtual int GetID() const noexcept { return -1; }
    virtual size_t GetSize() const noexcept = 0;
    virtual bool IsEmpty() const noexcept = 0;

    virtual Organism & At(size_t org_id) = 0;
    virtual const Organism & At(size_t org_id) const = 0;

    size_t size() const { return GetSize(); }
  };

  class OrgPosition;
  class ConstOrgPosition;

  template <typename DERIVED_T,
            typename ORG_T=Organism,
            typename CONTAINER_T=OrgContainer,
            typename INDEX_T=size_t>
  class OrgIterator_Interface {
  protected:
    emp::Ptr<CONTAINER_T> pop_ptr;
    INDEX_T pos;

    using this_t = OrgIterator_Interface<DERIVED_T, ORG_T, CONTAINER_T, INDEX_T>;
    using container_ptr_t = emp::Ptr<CONTAINER_T>;
    using pop_t = emp::match_const_t<Population, CONTAINER_T>;

    // Helper functions to be overloaded in derived classes.
    virtual void IncPosition() = 0;
    virtual void DecPosition() = 0;
    virtual void ShiftPosition(int=1) = 0;
    virtual void ToBegin() = 0;
    virtual void ToEnd() = 0;

    DERIVED_T & AsDerived() { return (DERIVED_T &) *this; }

  public:
    /// Constructor where you can optionally supply population pointer and position.
    OrgIterator_Interface(emp::Ptr<CONTAINER_T> _pop=nullptr, INDEX_T _pos=0)
      : pop_ptr(_pop), pos(_pos) { }

    /// Supply CONTAINER_T by reference instead of pointer.
    OrgIterator_Interface(CONTAINER_T & pop, INDEX_T _pos=0) : OrgIterator_Interface(&pop, _pos) {}

    /// Copy constructor
    OrgIterator_Interface(const this_t &) = default;

    /// Destructor
    virtual ~OrgIterator_Interface() { }

    /// Copy operator
    this_t & operator=(const this_t & in) = default;

    // Information direct from this iterator.
    INDEX_T Pos() const noexcept { return pos; };
    emp::Ptr<pop_t> PopPtr() noexcept {
      emp_assert(pop_ptr.template DynamicCast<pop_t>());
      return pop_ptr.template Cast<Population>();
    }
    emp::Ptr<const Population> PopPtr() const noexcept {
      emp_assert(pop_ptr.template DynamicCast<Population>());
      return pop_ptr.template Cast<const Population>();
    }
    emp::Ptr<const Population> ConstPopPtr() const noexcept { return PopPtr(); }

    Population & Pop() noexcept {
      emp_assert(PopPtr() != nullptr);
      return *PopPtr();
    }
    const Population & Pop() const noexcept {
      emp_assert(PopPtr() != nullptr);
      return *PopPtr();
    }

    // Shortcut definitions to retrieve information from the POPULATION.
    const std::string & PopName() const { emp_assert(pop_ptr); return pop_ptr->GetName(); }
    int PopID() const { emp_assert(pop_ptr); return pop_ptr->GetID(); }
    size_t PopSize() const { emp_assert(pop_ptr); return pop_ptr->GetSize(); }
    emp::Ptr<ORG_T> OrgPtr() { emp_assert(pop_ptr); return &pop_ptr->At(pos); }
    emp::Ptr<const Organism> OrgPtr() const { emp_assert(pop_ptr); return &pop_ptr->At(pos); }

    std::string ToString() const {
      return emp::to_string("{pop_ptr=", pop_ptr, ";pos=", pos, "}");
    }

    /// Convert to an OrgPosition
    [[nodiscard]] OrgPosition AsPosition();
    [[nodiscard]] ConstOrgPosition AsPosition() const;

    operator OrgPosition();
    operator ConstOrgPosition() const;

    DERIVED_T & SetContainer(CONTAINER_T & in) { pop_ptr = &in; return AsDerived(); }
    DERIVED_T & SetContainer(emp::Ptr<CONTAINER_T> in) { pop_ptr = in; return AsDerived(); }
    DERIVED_T & SetPos(INDEX_T in) { pos = in; return AsDerived(); }
    DERIVED_T & Set(emp::Ptr<CONTAINER_T> in_pop, INDEX_T in_pos) {
      pop_ptr = in_pop;
      pos = in_pos;
      return AsDerived();
    }

    /// Is this iterator currently in a legal state?
    bool IsValid() const { return !pop_ptr.IsNull() && pos < PopSize(); }

    /// Is the pointed-to cell occupied?
    bool IsEmpty() const { return IsValid() && OrgPtr()->IsEmpty(); }
    bool IsOccupied() const { return IsValid() && !OrgPtr()->IsEmpty(); }

    /// Is this position in the specified population?
    bool IsInPop(const Population & pop) const { return ConstPopPtr() == &pop; }

    /// Advance iterator to the next non-empty cell in the world.
    DERIVED_T & operator++() { IncPosition(); return AsDerived(); }

    /// Postfix++: advance iterator to the next non-empty cell in the world.
    DERIVED_T operator++(int) {
      DERIVED_T out = AsDerived();
      IncPosition();
      return out;
    }

    /// Backup iterator to the previos non-empty cell in the world.
    DERIVED_T & operator--() {
      DecPosition();
      return AsDerived();
    }

    /// Postfix--: Backup iterator to the previos non-empty cell in the world.
    DERIVED_T operator--(int) {
      DERIVED_T out = AsDerived();
      DecPosition();
      return out;
    }

    // Basic math operations...
    DERIVED_T operator+(int x) {
      DERIVED_T out = AsDerived();
      out.ShiftPosition(x);
      return out;
    }

    DERIVED_T operator-(int x) {
      DERIVED_T out = AsDerived();
      out.ShiftPosition(-x);
      return out;
    }

    // Compound math operations...
    DERIVED_T & operator+=(int x) {
      ShiftPosition(x);
      return AsDerived();
    }

    DERIVED_T & operator-=(int x) {
      ShiftPosition(-x);
      return AsDerived();
    }

    bool operator==(const DERIVED_T & in) const { return pop_ptr == in.pop_ptr && pos == in.pos; }
    bool operator!=(const DERIVED_T & in) const { return pop_ptr != in.pop_ptr || pos != in.pos; }
    bool operator< (const DERIVED_T & in) const {
      return (pop_ptr == in.pop_ptr) ? (pos <  in.pos) : (pop_ptr < in.pop_ptr);
    }
    bool operator> (const DERIVED_T & in) const { return in < *this; }
    bool operator<=(const DERIVED_T & in) const { return !(in < *this); }
    bool operator>=(const DERIVED_T & in) const { return !(*this < in); }

    // OLD VERSION : Two iterators don't have order when they are not in the same population.
    // bool operator< (const DERIVED_T & in) const { return pop_ptr == in.pop_ptr && pos <  in.pos; }
    // bool operator<=(const DERIVED_T & in) const { return pop_ptr == in.pop_ptr && pos <= in.pos; }
    // bool operator> (const DERIVED_T & in) const { return pop_ptr == in.pop_ptr && pos >  in.pos; }
    // bool operator>=(const DERIVED_T & in) const { return pop_ptr == in.pop_ptr && pos >= in.pos; }

    /// Return a reference to the organism pointed to by this iterator; may advance iterator.
    ORG_T & operator*() {
      emp_assert(IsValid(), pop_ptr, pos, PopSize());  // Make sure we're not outside of the vector.
      return *(OrgPtr());
    }

    /// Return a const reference to the organism pointed to by this iterator.
    /// Note that since this version is const, it will NOT adjust the iterator.
    const Organism & operator*() const { emp_assert(IsValid()); return *(OrgPtr()); }

    /// Allow iterator to be used as a pointer.
    emp::Ptr<ORG_T> operator->() {
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

    /// Is this Iterator pointing to a valid cell in the world?
    operator bool() const { return pos < PopSize() && IsOccupied(); }

    /// Iterators can be automatically converted to a pointer to the organism they refer to.
    operator emp::Ptr<ORG_T>() { emp_assert(IsValid()); return OrgPtr(); }

    /// Return an iterator pointing to the first occupied cell in the world.
    DERIVED_T begin() const { DERIVED_T out( (DERIVED_T &) *this); out.ToBegin(); return out; }

    /// Return an iterator pointing to just past the end of the world.
    DERIVED_T end() const { DERIVED_T out( (DERIVED_T &) *this); out.ToEnd(); return out; }
  };

  /// An indication of an Organism's position (including its population) that allows that Organism
  /// to be read AND to be modified.
  class OrgPosition : public OrgIterator_Interface<OrgPosition> {
  protected:
    using base_t = OrgIterator_Interface<OrgPosition>;

    void IncPosition() override { emp_error("IncPosition(shift_size) not defined in OrgPosition."); }
    void DecPosition() override { emp_error("DecPosition(shift_size) not defined in OrgPosition."); }
    void ShiftPosition(int=1) override { emp_error("ShiftPosition(shift_size) not defined in OrgPosition."); }
    void ToBegin() override { emp_error("ToBegin() not defined in OrgPosition."); }
    void ToEnd() override { emp_error("ToEnd() not defined in OrgPosition."); }

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

  /// An indication of an Organism's position (including its population) that allows that Organism
  /// to be read, but does NOT allow it to be modified.
  class ConstOrgPosition : public OrgIterator_Interface<ConstOrgPosition, const Organism> {
  protected:
    using base_t = OrgIterator_Interface<ConstOrgPosition, const Organism>;

    void IncPosition() override { emp_error("IncPosition(shift_size) not defined in ConstOrgPosition."); }
    void DecPosition() override { emp_error("DecPosition(shift_size) not defined in ConstOrgPosition."); }
    void ShiftPosition(int=1) override { emp_error("ShiftPosition(shift_size) not defined in ConstOrgPosition."); }
    void ToBegin() override { emp_error("ToBegin() not defined in ConstOrgPosition."); }
    void ToEnd() override { emp_error("ToEnd() not defined in ConstOrgPosition."); }

  public:
    /// Constructor where you can optionally supply population pointer and position.
    ConstOrgPosition(emp::Ptr<Population> _pop=nullptr, size_t _pos=0) : base_t(_pop, _pos) { ; }

    /// Supply Population by reference instead of pointer.
    ConstOrgPosition(Population & pop, size_t _pos=0) : ConstOrgPosition(&pop, _pos) {}

    /// Copy constructor
    ConstOrgPosition(const ConstOrgPosition &) = default;

    /// Copy operator
    ConstOrgPosition & operator=(const ConstOrgPosition & in) = default;
  };


  // -------------------------------------------------- //
  // --  OrgIterator_Interface function definitions  -- //
  // -------------------------------------------------- //

  template <typename DT, typename OT, typename CT, typename IT>
  [[nodiscard]] OrgPosition OrgIterator_Interface<DT,OT,CT,IT>::AsPosition() {
    return OrgPosition(PopPtr(), Pos());
  }

  template <typename DT, typename OT, typename CT, typename IT>
  [[nodiscard]] ConstOrgPosition OrgIterator_Interface<DT,OT,CT,IT>::AsPosition() const {
    return OrgPosition(PopPtr(), Pos());
  }

  template <typename DT, typename OT, typename CT, typename IT>
  OrgIterator_Interface<DT,OT,CT,IT>::operator OrgPosition() { return AsPosition(); }

  template <typename DT, typename OT, typename CT, typename IT>
  OrgIterator_Interface<DT,OT,CT,IT>::operator ConstOrgPosition() const { return AsPosition(); }

}

#endif
