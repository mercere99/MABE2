/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  DirectEncoding.h
 *  @brief An Organism class for evolutionary algorithms with simple, direct encodings.
 *  @note Status: PLANNING
 */

#ifndef MABE_DIRECT_ENCODING_H
#define MABE_DIRECT_ENCODING_H

#include "Organism.h"

namespace mabe {

  template <typename T>
  class DirectEncoding : public Organism {
  protected:
    using this_t = DirectEncoding<T>;
    T data;

  public:
    DirectEncoding() { ; }
    DirectEncoding(const DirectEncoding &) = default;
    DirectEncoding(DirectEncoding &&) = default;
    DirectEncoding(const T & in) : data(in) { ; }
    ~DirectEncoding() { ; }

    /// Use the default constructor for cloning.
    emp::Ptr<Organism> Clone() const override { return emp::NewPtr<this_t>(*this); }

    /// Use "to_string" as the default printing mechanism.
    std::string ToString() override { return emp::to_string(data); }

    /// Don't do mutations unless a mutate function has been set.
    int Mutate(emp::Random&) override {
      emp_assert(false, "No default Mutate() available."); return -1;
    }

    /// A direct encoding always uses itself as the output.
    void GenerateOutput(const std::string & output_name="result", size_t=0) override {
      var_map.Set<T>(output_name, data);
    }

    /// Request output type (multiple types are possible); default to unknown.
    /// Argument is the output ID.
    virtual emp::TypeID GetOutputType(size_t=0) override { return emp::GetTypeID<T>(); }
  };

}

#endif
