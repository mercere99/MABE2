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

namespace mabe {

  // Base class for all genome types.
  class Genome : public BitVector {

  };

}

#endif
