/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  OrgNK.h
 *  @brief An NK-model based organism (consisting of bits)
 *  @note Status: PLANNING
 */

#ifndef MABE_DIRECT_ENCODING_H
#define MABE_DIRECT_ENCODING_H

#include "../core/Organism.h"

#include "tools/BitVector.h"

namespace mabe {

  class OrgNK : public Organism {
  protected:
    emp::BitVector bits;

  public:
    OrgNK() { ; }
    OrgNK(const OrgNK &) = default;
    OrgNK(OrgNK &&) = default;
    OrgNK(const emp::BitVector & in) : bits(in) { ; }
    OrgNK(size_t N) : bits(N) { ; }
    ~OrgNK() { ; }

    /// Use the default constructor for cloning.
    emp::Ptr<Organism> Clone() override { return emp::NewPtr<OrgNK>(*this); }

    /// Use "to_string" to convert.
    std::string ToString() override { return emp::to_string(bits); }

    /// Don't do mutations unless a mutate function has been set.
    int Mutate(emp::Random & random) override {
      if (bits.size() == 0) return 0;
      for (size_t i = 0; i < 3; i++) {
        size_t pos = random.GetUInt(bits.size());
        bits[pos] = random.P(0.5);
      }
      return 3;
    }

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

}

#endif
