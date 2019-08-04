/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  BitsOrg.h
 *  @brief An organism consisting of a series of bits.
 *  @note Status: ALPHA
 */

#ifndef MABE_BITS_ORGANISM_H
#define MABE_BITS_ORGANISM_H

#include "../core/Organism.h"
#include "../core/OrganismType_Wrapper.h"

#include "tools/BitVector.h"
#include "tools/random_utils.h"

namespace mabe {

  class BitsOrg : public Organism {
    friend class BitsOrgType;
  protected:
    emp::BitVector bits;

  public:
    BitsOrg(emp::Ptr<OrganismType> ptr) : Organism(ptr) { ; }
    BitsOrg(const BitsOrg &) = default;
    BitsOrg(BitsOrg &&) = default;
    BitsOrg(const emp::BitVector & in, emp::Ptr<OrganismType> ptr) : Organism(ptr), bits(in) { ; }
    BitsOrg(size_t N, emp::Ptr<OrganismType> ptr) : Organism(ptr), bits(N) { ; }
    ~BitsOrg() { ; }

    /// Use "to_string" to convert.
    std::string ToString() override { return emp::to_string(bits); }

    /// Just use the bit sequence as the output.
    void GenerateOutput(const std::string & output_name="result", size_t=0) override {
      var_map.Set<emp::BitVector>(output_name, bits);
    }

    /// Request output type (multiple types are possible); default to unknown.
    /// Argument is the output ID.
    virtual emp::TypeID GetOutputType(size_t=0) override {
      return emp::GetTypeID<emp::BitVector>();
    }
  };

  class BitsOrgType : public OrganismType_Wrapper<BitsOrg> {
  private:
    size_t N;  ///< Number of bits in this type of organism.
    using base_t = OrganismType_Wrapper<BitsOrg>;
  public:
    BitsOrgType(const std::string & in_name) : base_t(in_name) { ; }

    emp::Ptr<Organism> MakeOrganism(emp::Random & random) const override {
      auto org_ptr = emp::NewPtr<BitsOrg>(N, this);
      Randomize(*org_ptr, random);
      return org_ptr;
    }

    size_t Mutate(Organism & org, emp::Random & random) const override {
      auto & bits = ConvertOrg(org).bits;

      if (bits.size() == 0) return 0;
      for (size_t i = 0; i < 3; i++) {
        size_t pos = random.GetUInt(bits.size());
        bits[pos] = random.P(0.5);
      }
      return 3;
    }

    void Randomize(Organism & org, emp::Random & random) const override {
      emp::RandomizeBitVector(ConvertOrg(org).bits, random, 0.5);
    }

    std::ostream & Print(Organism & org, std::ostream & os) const override {
      emp_assert(&(org.GetType()) == this);
      os << org.ToString();
      return os;
    }

  };

}

#endif
