/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
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

#include "../Emplode/EmplodeType.hpp"

#include "Organism.hpp"
#include "OrgIterator.hpp"

namespace mabe {

  using emplode::EmplodeType;

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

  /// A Population maintains a collection of organisms.  It is derived from EmplodeType so that it
  /// can be easily used in the MABE scripting language.
  class Population : public OrgContainer {
    friend class MABEBase;
  private:
    std::string name="";                   ///< Unique name for this population.
    size_t pop_id = (size_t) -1;           ///< Position in world of this population.
    emp::vector<emp::Ptr<Organism>> orgs;  ///< Info on all organisms in this population.
    size_t num_orgs = 0;                   ///< How many LIVING organisms are in this population?

    /// Pointer to layout used in data maps of orgs.
    emp::Ptr<emp::DataLayout> data_layout_ptr = nullptr; 

    /// Organism to fill in empty cells (does have data map!)
    emp::Ptr<Organism> empty_org = nullptr; 

    std::function<OrgPosition(Organism &, OrgPosition)> place_birth_fun;
    std::function<OrgPosition(Organism &)> place_inject_fun;
    std::function<OrgPosition(OrgPosition)> find_neighbor_fun;

  public:
    using iterator_t = PopIterator;
    using const_iterator_t = ConstPopIterator;

    Population() { emp_assert(false, "Do not use default constructor on Population!"); }
    Population(const std::string & in_name,
               size_t in_id,
               size_t pop_size=0,
               emp::Ptr<Organism> in_empty=nullptr)
      : name(in_name), pop_id(in_id), empty_org(in_empty)
    {
      orgs.resize(pop_size, empty_org);
    }

    // All organism moving/copying must be tracked and done through MABE object.
    Population(const Population & in_pop) = delete;
    Population(Population &&) = delete;
    Population & operator=(const Population & in_pop) = delete;
    Population & operator=(Population &&) = delete;

    ~Population() { emp_assert(num_orgs==0, "Population should be cleaned up before deletion."); }

    std::string GetName() const override { return name; }
    int GetID() const noexcept override { return pop_id; }
    size_t GetSize() const noexcept override { return orgs.size(); }
    size_t GetNumOrgs() const noexcept { return num_orgs; }
    bool IsEmpty() const noexcept override { return num_orgs == 0; }

    bool HasDataLayout() const { return data_layout_ptr; }
    emp::DataLayout & GetDataLayout() noexcept { 
      emp_assert(HasDataLayout());
      return *data_layout_ptr;
    }
    const emp::DataLayout & GetDataLayout() const noexcept {
      emp_assert(HasDataLayout());
      return *data_layout_ptr;
    }

    bool IsValid(size_t pos) const { return pos < orgs.size(); }
    bool IsEmpty(size_t pos) const { return IsValid(pos) && orgs[pos]->IsEmpty(); }
    bool IsOccupied(size_t pos) const { return IsValid(pos) && !orgs[pos]->IsEmpty(); }

    void SetName(const std::string & in_name) { name = in_name; }
    void SetID(int in_id) noexcept { pop_id = in_id; }

    template <typename FUN_T> void SetPlaceBirthFun(FUN_T fun) { place_birth_fun = fun; }
    template <typename FUN_T> void SetPlaceInjectFun(FUN_T fun) { place_inject_fun = fun; }
    template <typename FUN_T> void SetFindNeighborFun(FUN_T fun) { find_neighbor_fun = fun; }

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

    OrgPosition PlaceBirth(Organism & org, OrgPosition ppos) { return place_birth_fun(org, ppos); }
    OrgPosition PlaceInject(Organism & org) { return place_inject_fun(org); }
    OrgPosition FindNeighbor(OrgPosition pos) { return find_neighbor_fun(pos); }

  private:  // ---== To be used by friend class MABEBase only! ==---

    void SetOrg(size_t pos, emp::Ptr<Organism> org_ptr) {
      emp_assert(pos < orgs.size());
      emp_assert(IsEmpty(pos));         // Must be valid and should not overwrite a living cell.
      emp_assert(!org_ptr->IsEmpty());  // Use ExtractOrg if you want to make a cell empty.
      orgs[pos] = org_ptr;
      org_ptr->SetPopulation(*this);
      if (!data_layout_ptr) data_layout_ptr = &org_ptr->GetDataMap().GetLayout();

      if ( &org_ptr->GetDataMap().GetLayout() != data_layout_ptr ) {
        emp::notify::Error("Trying to insert an organism into population '", name,
                           "' with the incorrect trait set.");
      }
      num_orgs++;
    }

    /// Remove (and return) the organism at pos, but don't delete it.
    [[nodiscard]] emp::Ptr<Organism> ExtractOrg(size_t pos) {
      emp_assert(pos < orgs.size());
      emp_assert(!empty_org.IsNull(), "Empty org must be provided before extraction.");
      emp::Ptr<Organism> out_org = orgs[pos];
      orgs[pos] = empty_org;
      if (!out_org->IsEmpty()) {
        num_orgs--;
        out_org->ClearPopulation(); // Alert organism that it is no longer part of this population.
      }
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
    // Setup member functions associated with population.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("ID", [](Population & target) { return target.GetID(); },
                             "Return the ID number for the population.");
      info.AddMemberFunction("NAME", [](Population & target) { return target.GetName(); },
                             "Return the name of the population.");
      info.AddMemberFunction("NUM_ORGS", [](Population & target) { return target.GetNumOrgs(); },
                             "Return the number of organisms in the population.");
      info.AddMemberFunction("SIZE", [](Population & target) { return target.GetSize(); },
                             "Return the capacity of the population.");
      info.AddMemberFunction("PTR", [](Population & target) { return (size_t) &target; },
                             "DEBUG: Give memory location of target.");
    }


    // ------ DEBUG FUNCTIONS ------
    bool OK() const {
      // We may have a handful of populations, but assume error if we have more than a million.
      if (pop_id > 1000000) {
        std::cerr << "WARNING: Invalid Population ID (pop_id = " << pop_id << ")" << std::endl;
        return false;
      }

      // We should never have more living organisms than slots in the population.
      if (num_orgs > orgs.size()) {
        std::cerr << "ERROR: Population " << pop_id << " size is " << orgs.size()
                  << " but num_orgs = " << num_orgs << std::endl;
        return false;
      }

      // Scan through the population and make sure every position is valid.
      size_t org_count = 0;
      for (size_t pos = 0; pos < orgs.size(); pos++) {
        // No vector positions should be NULL (use EmptyOrganism instead)
        if (orgs[pos].IsNull()) {
          std::cerr << "ERROR: Population " << pop_id << " as position " << pos
                    << " has null pointer instead of an organism." << std::endl;
          return false;
        }

        // Organisms should point back at this population.
        if (orgs[pos]->GetPopPtr() != this) {
          std::cerr << "ERROR: Population " << pop_id << " org# " << pos
                    << " does not point back at the correct population." << std::endl;
          return false;
        }

        // Count the number of living (non-empty) organisms as we go.
        if (!orgs[pos]->IsEmpty()) org_count++;
      }

      // Make sure we counted the correct number of organisms in the population.
      if (num_orgs != org_count) {
          std::cerr << "ERROR: Population " << pop_id << " has num_orgs = " << num_orgs
                    << ", but audit counts " << org_count << " orgs." << std::endl;
          return false;
      }

      // @CAO: If we have a cap on the population size, make sure we haven't crossed it?

      return true;
    }

    static std::string EMPGetTypeName() { return "mabe::Population"; }
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
