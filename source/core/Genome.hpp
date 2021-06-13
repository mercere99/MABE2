/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  Genome.hpp
 *  @brief Base genome representation for organisms.
 */

#ifndef MABE_GENOME_HPP
#define MABE_GENOME_HPP

#include "emp/bits/BitVector.hpp"
#include "emp/math/Random.hpp"
#include "emp/meta/TypeID.hpp"

namespace mabe {

  // Base class for all genome types.
  class GenomeBase : public emp::BitVector {
    /// Set all bits randomly, with a 50/50 probability.
    virtual GenomeBase & Randomize(emp::Random & random) = 0;

    /// Set all bits randomly, with a given probability of being a one.
    virtual GenomeBase & Randomize(emp::Random & random, const double p) = 0;

    /// Identify the locus type used in this genome.
    virtual emp::TypeID GetLocusType() const = 0;

    /// Test to see if this genome has a specified locus type.
    template <typename T>
    bool HasLocusType() const { return GetLocusType() == GetTypeID<T>(); }

    /// Get entry at a given indexof a specified type  (in steps of that type size)
    template <typename LOCUS_T>
    [[nodiscard]] LOCUS_T Get(const size_t index) {
      return emp::BitVector::GetValueAtIndex<LOCUS_T>(index) const;
    }

    /// Set entry at a given indexof a specified type  (in steps of that type size)
    template <typename LOCUS_T>
    void Set(const size_t index, LOCUS_T value) {
      emp::BitVector::SetValueAtIndex<LOCUS_T>(index, value);
    }

    /// Get entry at a given bit
    template <typename LOCUS_T>
    [[nodiscard]] LOCUS_T GetAtBit(const size_t index) {
      return emp::BitVector::GetValueAtBit<LOCUS_T>(index) const;
    }

    /// Set entry at a given bit
    template <typename LOCUS_T>
    void SetAtBit(const size_t index, LOCUS_T value) {
      emp::BitVector::SetValueAtBit<LOCUS_T>(index, value);
    }

    /// Get entry at a given byte
    template <typename LOCUS_T>
    [[nodiscard]] LOCUS_T GetAtByte(const size_t index) {
      return emp::BitVector::GetValueAtBit<LOCUS_T>(index*8) const;
    }

    /// Set entry at a given byte
    template <typename LOCUS_T>
    void SetAtByte(const size_t index, LOCUS_T value) {
      emp::BitVector::SetValueAtBit<LOCUS_T>(index*8, value);
    }
  };

  template <typename LOCUS_T>
  class Genome : public GenomeBase {
    using this_t = Genome<LOCUS_T>;

    /// Set all bits randomly, with a 50/50 probability.
    this_t & Randomize(emp::Random & random) override {
      emp::BitVector::Randomize(random);
      return *this;
    }

    /// Set all bits randomly, with a given probability of being a one.
    this_t & Randomize(emp::Random & random, const double p) override {
      emp::BitVector::Randomize(random, p);
      return *this;
    }

    emp::TypeID GetLocusType() const override { return emp::GetTypeID<LOCUS_T>(); }

    /// Get entry at a given index (in steps of that type size)
    [[nodiscard]] virtual LOCUS_T Get(const size_t index) {
      return emp::BitVector::GetValueAtIndex<LOCUS_T>(index) const;
    }

    /// Set entry at a given index (in steps of that type size)
    virtual void Set(const size_t index, LOCUS_T value) {
      emp::BitVector::SetValueAtIndex<LOCUS_T>(index, value);
    }
 
  };

}

#endif
