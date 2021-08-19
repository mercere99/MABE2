/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  Population.hpp
 *  @brief Container for a group of arbitrary MABE organisms.
 *
 *  Organisms in MABE are stored in Population objects.
 *  A single position in a Population object is described by a Population::Position.
 * 
 *  @todo Add a reverse iterator.
 *  @todo Fix operator-- which can go off of the beginning of the world.
 */

#ifndef MABE_POPULATION_H
#define MABE_POPULATION_H

#include <string>

#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"

#include "../config/ConfigType.hpp"

#include "Organism.hpp"
#include "OrgIterator.hpp"

namespace mabe {

  class PopIterator : public OrgIterator_Interface<PopIterator, Organism, Population> {
  protected:
    using base_t = OrgIterator_Interface<PopIterator, Organism, Population>;

    void IncPosition() override;
    void DecPosition() override;
    void ShiftPosition(int shift=1) override;
    void ToBegin() override;
    void ToEnd() override;
    void MakeValid();

  public:
    /// Constructor where you can optionally supply population pointer and position.
    PopIterator(emp::Ptr<Population> _pop=nullptr, size_t _pos=0) : base_t(_pop, _pos) { ; }

    /// Supply Population by reference instead of pointer.
    PopIterator(Population & pop, size_t _pos=0) : PopIterator(&pop, _pos) {}

    /// Copy constructor
    PopIterator(const PopIterator &) = default;

    /// Copy operator
    PopIterator & operator=(const PopIterator & in) = default;
  };

  class ConstPopIterator
  : public OrgIterator_Interface<ConstPopIterator, const Organism, const Population> {
  protected:
    using base_t = OrgIterator_Interface<ConstPopIterator, const Organism, const Population>;

    void IncPosition() override;
    void DecPosition() override;
    void ShiftPosition(int shift=1) override;
    void ToBegin() override;
    void ToEnd() override;
    void MakeValid();

  public:
    /// Constructor where you can optionally supply population pointer and position.
    ConstPopIterator(emp::Ptr<const Population> _pop=nullptr, size_t _pos=0) : base_t(_pop, _pos) { ; }

    /// Supply Population by reference instead of pointer.
    ConstPopIterator(const Population & pop, size_t _pos=0) : ConstPopIterator(&pop, _pos) {}

    /// Copy constructor
    ConstPopIterator(const ConstPopIterator &) = default;

    /// Copy operator
    ConstPopIterator & operator=(const ConstPopIterator & in) = default;
  };

  /// A Population maintains a collection of organisms.  It is derived from ConfigType so that it
  /// can be easily used in the MABE scripting language.
  class Population : public ConfigType, public OrgContainer {
    friend class MABEBase;
  private:
    std::string name="";                    ///< Unique name for this population.
    size_t pop_id = (size_t) -1;            ///< Position in world of this population.
    emp::vector<emp::Ptr<Organism>> orgs;   ///< Info on all organisms in this population.
    size_t num_orgs = 0;                    ///< How many living organisms are in this population?
    size_t max_orgs = (size_t) -1;          ///< Maximum number of orgs allowed in population.

    emp::Ptr<Organism> empty_org = nullptr; ///< Organism to fill in empty cells (does have data map!)

  public:
    using iterator_t = PopIterator;
    using const_iterator_t = ConstPopIterator;

  public:
    Population() { emp_assert(false, "Do not use default constructor on Population!"); }
    Population(const std::string & in_name,
               size_t in_id,
               size_t pop_size=0,
               emp::Ptr<Organism> in_empty=nullptr)
      : name(in_name), pop_id(in_id), empty_org(in_empty)
    {
      orgs.resize(pop_size, empty_org);
    }
    Population(const Population & in_pop)
      : name(in_pop.name), pop_id(in_pop.pop_id), orgs(in_pop.orgs.size())
      , num_orgs(in_pop.num_orgs), max_orgs(in_pop.max_orgs)
      , empty_org(in_pop.empty_org)
    {
      emp_assert(in_pop.OK());
      for (size_t i = 0; i < orgs.size(); i++) {
        if (in_pop.orgs[i]->IsEmpty()) {       // Make sure we always use local empty organism.
          emp_assert(!empty_org.IsNull(), "Empty organisms must be set before they can be used!");
          orgs[i] = empty_org;
        } else {                              // Otherwise clone the organism.
          orgs[i] = in_pop.orgs[i]->CloneOrganism();
        }
      }
      emp_assert(OK());
    }

    // Populations can be copied, but should not be moved to maintain correct empty orgs.
    Population(Population &&) = delete;
    Population & operator=(Population &&) = delete;

    ~Population() { for (auto x : orgs) if (!x->IsEmpty()) x.Delete(); }

    std::string GetName() const override { return name; }
    int GetID() const noexcept override { return pop_id; }
    size_t GetSize() const noexcept override { return orgs.size(); }
    size_t GetNumOrgs() const noexcept { return num_orgs; }

    bool IsValid(size_t pos) const { return pos < orgs.size(); }
    bool IsEmpty(size_t pos) const { return IsValid(pos) && orgs[pos]->IsEmpty(); }
    bool IsOccupied(size_t pos) const { return IsValid(pos) && !orgs[pos]->IsEmpty(); }

    void SetID(int in_id) noexcept { pop_id = in_id; }

    Organism & operator[](size_t org_id) { return *(orgs[org_id]); }
    const Organism & operator[](size_t org_id) const { return *(orgs[org_id]); }
    Organism & At(size_t org_id) override { return *(orgs[org_id]); }
    const Organism & At(size_t org_id) const override { return *(orgs[org_id]); }

    iterator_t begin() { return iterator_t(this, 0); }
    const_iterator_t begin() const { return const_iterator_t(this, 0); }
    iterator_t end() { return iterator_t(this, GetSize()); }
    const_iterator_t end() const { return const_iterator_t(this, GetSize()); }

    iterator_t IteratorAt(size_t pos) { return iterator_t(this, pos); }
    const_iterator_t ConstIteratorAt(size_t pos) const { return const_iterator_t(this, pos); }

    /// Required SetupConfig function; for now population don't have any config optons.
    void SetupConfig() override { }

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
      emp_assert(!empty_org.IsNull(), "Empty org must be provided before extraction.");
      emp::Ptr<Organism> out_org = orgs[pos];
      orgs[pos] = empty_org;
      if (!out_org->IsEmpty()) num_orgs--;
      return out_org;
    }

    /// Resize a population; should only be called from world after removed orgs are deleted.
    Population & Resize(size_t new_size) {
      emp_assert(num_orgs == 0);
      emp_assert(new_size <= orgs.size() || !empty_org.IsNull(),
                 "Population resize can only increase size if empty_org is provided.",
                 new_size, orgs.size());

      // Resize the population, adding in empty cells to any new spaces.
      orgs.resize(new_size, empty_org);

      return *this;
    }

    /// Add an empty position to the end of the population (and return an iterator to it)
    iterator_t PushEmpty() {
      emp_assert(!empty_org.IsNull(),
                 "Population can only PushEmpty() if empty_org is provided.");
      size_t pos = orgs.size();
      orgs.resize(orgs.size()+1, empty_org);
      return iterator_t(this, pos);
    }

    /// Setup the organism to be used as "empty" (Managed externally, usually by MABE conroller.)
    void SetEmpty(emp::Ptr<Organism> in_empty) { empty_org = in_empty; }

  public:
    // ------ DEBUG FUNCTIONS ------
    bool OK() const {
      // We will usually have a handful of popoulations; assume error if we have more than a billion.
      if (pop_id > 1000000000) {
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


  // -------------------------------
  // --  PopIterator Definitions  --
  // -------------------------------

  void PopIterator::IncPosition() {
    emp_assert(pop_ptr);
    emp_assert(pos < pop_ptr->GetSize(), pos, pop_ptr->GetSize());
    ++pos;
  }
  void PopIterator::DecPosition() {
    emp_assert(pop_ptr);
    emp_assert(pos > 0, pos, pop_ptr->GetSize());
    --pos;
  }
  void PopIterator::ShiftPosition(int shift) {
    const int new_pos = shift + (int) pos;
    emp_assert(pop_ptr);
    emp_assert(new_pos >= 0 && new_pos <= (int) pop_ptr->GetSize(), new_pos, pop_ptr->GetSize());
    pos = (size_t) new_pos;
  }
  void PopIterator::ToBegin() { pos = 0; }
  void PopIterator::ToEnd() { pos = pop_ptr->GetSize(); }
  void PopIterator::MakeValid() {
    // If we moved past the end, make this the end iterator.
    if (pos > pop_ptr->GetSize()) ToEnd();
  }



  // ------------------------------------
  // --  ConstPopIterator Definitions  --
  // ------------------------------------

  void ConstPopIterator::IncPosition() {
    emp_assert(pop_ptr);
    emp_assert(pos < pop_ptr->GetSize(), pos, pop_ptr->GetSize());
    ++pos;
  }
  void ConstPopIterator::DecPosition() {
    emp_assert(pop_ptr);
    emp_assert(pos > 0, pos, pop_ptr->GetSize());
    --pos;
  }
  void ConstPopIterator::ShiftPosition(int shift) {
    const int new_pos = shift + (int) pos;
    emp_assert(pop_ptr);
    emp_assert(new_pos >= 0 && new_pos <= (int) pop_ptr->GetSize(), new_pos, pop_ptr->GetSize());
    pos = (size_t) new_pos;
  }
  void ConstPopIterator::ToBegin() { pos = 0; }
  void ConstPopIterator::ToEnd() { pos = pop_ptr->GetSize(); }
  void ConstPopIterator::MakeValid() {
    // If we moved past the end, make this the end iterator.
    if (pos > pop_ptr->GetSize()) ToEnd();
  }

}

#endif
