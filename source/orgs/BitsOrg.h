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
#include "../core/OrganismManager_Wrapper.h"

#include "tools/BitVector.h"
#include "tools/random_utils.h"

namespace mabe {

  class BitsOrg : public Organism {
    friend class BitsOrgManager;
  protected:
    emp::BitVector bits;

  public:
    BitsOrg(emp::Ptr<const OrganismManager> ptr) : Organism(ptr) {
      emp_assert(!manager_ptr.IsNull());
    }
    BitsOrg(const BitsOrg &) = default;
    BitsOrg(BitsOrg &&) = default;
    BitsOrg(const emp::BitVector & in, emp::Ptr<const OrganismManager> ptr)
    : Organism(ptr), bits(in) {
      emp_assert(!manager_ptr.IsNull());
    }
    BitsOrg(size_t N, emp::Ptr<const OrganismManager> ptr) : Organism(ptr), bits(N) {
      emp_assert(!manager_ptr.IsNull());
    }
    ~BitsOrg() { ; }

    /// Use "to_string" to convert.
    std::string ToString() override { return emp::to_string(bits); }

    /// Just use the bit sequence as the output.
    void GenerateOutput(const std::string & output_name="result", size_t=0) override {
      var_map.Set<emp::BitVector>(output_name, bits);
    }

    /// Request output type (multiple types are possible); default to unknown.
    /// Argument is the output ID.
    emp::TypeID GetOutputType(size_t=0) override {
      return emp::GetTypeID<emp::BitVector>();
    }

    /// Setup this organism to load from config.
    void SetupConfig(ConfigScope & config_scope) override {
      config_scope.LinkFuns<size_t>([this](){ return bits.size(); },
                                    [this](const size_t & N){ return bits.Resize(N); },
                                    "N", "Number of bits in organism", 1);
    }
  };

  class BitsOrgManager : public OrganismManager_Wrapper<BitsOrg> {
  private:
    size_t N;  ///< Number of bits in this type of organism.
    using base_t = OrganismManager_Wrapper<BitsOrg>;
  public:
    BitsOrgManager(const std::string & in_name) : base_t(in_name) { ; }

    std::string GetTypeName() const override { return "BitsOrgManager"; }

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
      emp_assert(&(org.GetManager()) == this);
      os << org.ToString();
      return os;
    }

  };

}

#endif
