/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  MABE.cpp
 *  @brief Tests for ConfigLexer with various types and edge cases 
 */

// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "config/ConfigLexer.hpp"

TEST_CASE("ConfigLexerGetter", "[config]"){
  {
    mabe::ConfigLexer config_lexer;

    emp::Token token00 = 0;
    bool result = config_lexer.IsID(token00);
    REQUIRE(result == true);

    emp::Token token01 = 0;
    bool result = config_lexer.IsNumber(token01);
    REQUIRE(result == true);

    emp::Token token02 = 0;
    bool result = config_lexer.IsString(token02);
    REQUIRE(result == true);

    emp::Token token03 = 0;
    bool result = config_lexer.IsChar(token03);
    REQUIRE(result == true);

    emp::Token token04 = 0;
    bool result = config_lexer.IsDots(token04);
    REQUIRE(result == true);

    emp::Token token05 = 0;
    bool result = config_lexer.IsSymbol(token05);
    REQUIRE(result == true);
  }
}