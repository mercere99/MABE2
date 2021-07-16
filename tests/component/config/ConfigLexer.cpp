/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  MABE.cpp
 *  @brief Tests for ConfigLexer with various types and edge cases 
 */

#include <functional>
#include <string>

// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "config/ConfigLexer.hpp"
#include "core/MABE.hpp"

TEST_CASE("ConfigLexerBasic", "[config]"){
  {
    mabe::ConfigLexer config_lexer();
  }
}