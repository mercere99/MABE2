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

// Test ID tokens
TEST_CASE("ConfigLexerID", "[config]"){
  {
    mabe::ConfigLexer config_lexer00;
    std::string s00 = "a";
    emp::Token token00 = config_lexer00.Process(s00);
    bool result00 = config_lexer00.IsID(token00);
    REQUIRE(result00 == true);

    std::string s01 = "_a_0_";
    emp::Token token01 = config_lexer00.Process(s01);
    bool result01 = config_lexer00.IsID(token01);
    REQUIRE(result01 == true);

    std::string s02 = "0a";
    emp::Token token02 = config_lexer00.Process(s02);
    bool result02 = config_lexer00.IsID(token02);
    REQUIRE(result02 == false);
  }
}

// Test Number tokens
TEST_CASE("ConfigLexerNumber", "[config]"){
  {
    mabe::ConfigLexer config_lexer01;

    std::string s00 = "09";
    emp::Token token00 = config_lexer01.Process(s00);
    bool result00 = config_lexer01.IsNumber(token00);
    REQUIRE(result00 == true);

    // Test failing, negative numbers not accepted
    std::string s01 = "-1";
    emp::Token token01 = config_lexer01.Process(s01);
    bool result01 = config_lexer01.IsNumber(token01);
    REQUIRE(result01 == true);

    std::string s02 = ".9";
    emp::Token token02 = config_lexer01.Process(s02);
    bool result02 = config_lexer01.IsNumber(token02);
    REQUIRE(result02 == false);
  }
}


// Test String tokens
TEST_CASE("ConfigLexerString", "[config]"){
  {
    mabe::ConfigLexer config_lexer03;

    // Test failing, empty string not accepted 
    std::string s00 = " \" \" ";
    emp::Token token00 = config_lexer03.Process(s00);
    bool result00 = config_lexer03.IsString(token00);
    REQUIRE(result00 == true);

    std::string s01 = " \"a \" ";
    emp::Token token01 = config_lexer03.Process(s01);
    bool result01 = config_lexer03.IsString(token01);
    REQUIRE(result01 == true);

    std::string s02 = " \"0 \" ";
    emp::Token token02 = config_lexer03.Process(s02);
    bool result02 = config_lexer03.IsString(token02);
    REQUIRE(result02 == false);
  }
}


// Test Char tokens
TEST_CASE("ConfigLexerChar", "[config]"){
  {
    mabe::ConfigLexer config_lexer04;

    std::string s00 = "'a'";
    emp::Token token00 = config_lexer04.Process(s00);
    bool result00 = config_lexer04.IsChar(token00);
    REQUIRE(result00 == true);

    std::string s01 = "'9'";
    emp::Token token01 = config_lexer04.Process(s01);
    bool result01 = config_lexer04.IsChar(token01);
    REQUIRE(result01 == true);

    std::string s02 = "''a' ";
    emp::Token token02 = config_lexer04.Process(s02);
    bool result02 = config_lexer04.IsChar(token02);
    REQUIRE(result02 == false);
  }
}

// Test Dot tokens
TEST_CASE("ConfigLexerDots", "[config]"){
  {
    mabe::ConfigLexer config_lexer05;

    std::string s00 = ".";
    emp::Token token00 = config_lexer05.Process(s00);
    bool result00 = config_lexer05.IsDots(token00);
    REQUIRE(result00 == true);

    // Test failing, ._ accepted as dots
    std::string s01 = "._";
    emp::Token token01 = config_lexer05.Process(s01);
    bool result01 = config_lexer05.IsDots(token01);
    REQUIRE(result01 == false);

    std::string s02 = ".a";
    emp::Token token02 = config_lexer05.Process(s02);
    bool result02 = config_lexer05.IsDots(token02);
    REQUIRE(result02 == false);
  }
}

// Test Symbol tokens
TEST_CASE("ConfigLexerSymbol", "[config]"){
  {
    mabe::ConfigLexer config_lexer06;

    std::string s00 = "!=";
    emp::Token token00 = config_lexer06.Process(s00);
    bool result00 = config_lexer06.IsSymbol(token00);
    REQUIRE(result00 == true);

    std::string s01 = "::";
    emp::Token token01 = config_lexer06.Process(s01);
    bool result01 = config_lexer06.IsSymbol(token01);
    REQUIRE(result01 == true);

    std::string s02 = ".";
    emp::Token token02 = config_lexer06.Process(s02);
    bool result02 = config_lexer06.IsSymbol(token02);
    REQUIRE(result02 == false);
  }
}