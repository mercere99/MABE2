/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigLexer.cpp
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
    // Create ConfigLexer object
    mabe::ConfigLexer config_lexer00;

    std::string s00 = "a";
    emp::Token token00 = config_lexer00.Process(s00);
    bool result00 = config_lexer00.IsID(token00);
    CHECK(result00 == true);

    std::string s01 = "_a_0_";
    emp::Token token01 = config_lexer00.Process(s01);
    bool result01 = config_lexer00.IsID(token01);
    CHECK(result01 == true);

    std::string s02 = "0a";
    emp::Token token02 = config_lexer00.Process(s02);
    bool result02 = config_lexer00.IsID(token02);
    CHECK(result02 == false);
  }
}

// Test Number tokens
TEST_CASE("ConfigLexerNumber", "[config]"){
  {
    // Create ConfigLexer object
    mabe::ConfigLexer config_lexer01;

    std::string s00 = "09";
    emp::Token token00 = config_lexer01.Process(s00);
    bool result00 = config_lexer01.IsNumber(token00);
    CHECK(result00 == true);

    // Test failing!! negative numbers not accepted
    std::string s01 = "-1";
    emp::Token token01 = config_lexer01.Process(s01);
    bool result01 = config_lexer01.IsNumber(token01);
    CHECK_FALSE(result01);

    std::string s02 = "0.9";
    emp::Token token02 = config_lexer01.Process(s02);
    bool result02 = config_lexer01.IsNumber(token02);
    CHECK(result02 == true);

    std::string s03 = ".9";
    emp::Token token03 = config_lexer01.Process(s03);
    bool result03 = config_lexer01.IsNumber(token03);
    CHECK(result03 == false);
  }
}


// Test String tokens
TEST_CASE("ConfigLexerString", "[config]"){
  {
    // Create ConfigLexer object
    mabe::ConfigLexer config_lexer02;

    std::string s00 = "\"\"";
    emp::Token token00 = config_lexer02.Process(s00);
    bool result00 = config_lexer02.IsString(token00);
    CHECK(result00 == true);

    std::string s01 = "\" \"";
    emp::Token token01 = config_lexer02.Process(s01);
    bool result01 = config_lexer02.IsString(token01);
    CHECK(result01 == true);

    std::string s02 = "\"a\"";
    emp::Token token02 = config_lexer02.Process(s02);
    bool result02 = config_lexer02.IsString(token02);
    CHECK(result02 == true);

    std::string s03 = "\"0 \"";
    emp::Token token03 = config_lexer02.Process(s03);
    bool result03 = config_lexer02.IsString(token03);
    CHECK(result03 == true);

    std::string s04 = "\"(3 + (x / 4))\"";
    emp::Token token04 = config_lexer02.Process(s04);
    bool result04 = config_lexer02.IsString(token04);
    CHECK(result04 == true);
  }
}

// Test Char tokens
TEST_CASE("ConfigLexerChar", "[config]"){
  {
    // Create ConfigLexer object
    mabe::ConfigLexer config_lexer03;

    std::string s00 = "'a'";
    emp::Token token00 = config_lexer03.Process(s00);
    bool result00 = config_lexer03.IsChar(token00);
    CHECK(result00 == true);

    std::string s01 = "'9'";
    emp::Token token01 = config_lexer03.Process(s01);
    bool result01 = config_lexer03.IsChar(token01);
    CHECK(result01 == true);

    std::string s02 = "''a'";
    emp::Token token02 = config_lexer03.Process(s02);
    bool result02 = config_lexer03.IsChar(token02);
    CHECK(result02 == false);

    emp::TokenStream token_stream = config_lexer03.Tokenize("testing", "name");
    CHECK(token_stream.size() == 1);
    CHECK_FALSE(config_lexer03.IsChar(token_stream.Get(0)));
  }
}

// Test Dot tokens
TEST_CASE("ConfigLexerDots", "[config]"){
  {
    // Create ConfigLexer object
    mabe::ConfigLexer config_lexer04;

    std::string s00 = ".";
    emp::Token token00 = config_lexer04.Process(s00);
    bool result00 = config_lexer04.IsDots(token00);
    CHECK(result00 == true);
    
    // Test more than just first symbol to ensure ._ is not just dots 
    std::string s01 = "._";
    emp::Token token01 = config_lexer04.Process(s01);
    emp::TokenStream token_stream01 = config_lexer04.Tokenize(s01, "name");
    CHECK(token_stream01.size() > 1);
    CHECK(config_lexer04.IsDots(token_stream01.Get(0)));

    std::string s02 = "...";
    emp::Token token02 = config_lexer04.Process(s02);
    bool result02 = config_lexer04.IsDots(token02);
    CHECK(result02 == true);

    std::string s03 = "..";
    emp::Token token03 = config_lexer04.Process(s03);
    bool result03 = config_lexer04.IsDots(token03);
    CHECK(result03 == true);
    
    std::string s04 = "a";
    emp::Token token04 = config_lexer04.Process(s04);
    bool result04 = config_lexer04.IsDots(token04);
    CHECK(result04 == false);
    
    std::string s05 = ".a";
    emp::Token token05 = config_lexer04.Process(s05);
    emp::TokenStream token_stream05 = config_lexer04.Tokenize(s05, "name");
    CHECK(token_stream05.size() > 1);
    CHECK(config_lexer04.IsDots(token_stream05.Get(0)));
   
    std::string s06 = "a.";
    emp::Token token06 = config_lexer04.Process(s06);
    bool result06 = config_lexer04.IsDots(token06);
    CHECK(result06 == false);
  }
}

// Test Symbol tokens
TEST_CASE("ConfigLexerSymbol", "[config]"){
  {
    // Create ConfigLexer object
    mabe::ConfigLexer config_lexer05;

    std::string s00 = ".";
    emp::Token token00 = config_lexer05.Process(s00);
    bool result00 = config_lexer05.IsSymbol(token00);
    CHECK(result00 == false);

    std::string s01 = "::";
    emp::Token token01 = config_lexer05.Process(s01);
    bool result01 = config_lexer05.IsSymbol(token01);
    CHECK(result01 == true);

    std::string s02 = "==";
    emp::Token token02 = config_lexer05.Process(s02);
    bool result02 = config_lexer05.IsSymbol(token02);
    CHECK(result02 == true);

    std::string s03 = "!=";
    emp::Token token03 = config_lexer05.Process(s02);
    bool result03 = config_lexer05.IsSymbol(token03);
    CHECK(result03 == true);

    std::string s04 = "<=";
    emp::Token token04 = config_lexer05.Process(s04);
    bool result04 = config_lexer05.IsSymbol(token04);
    CHECK(result04 == true);

    std::string s05 = ">=";
    emp::Token token05 = config_lexer05.Process(s05);
    bool result05 = config_lexer05.IsSymbol(token05);
    CHECK(result05 == true);

    std::string s06 = "->";
    emp::Token token06 = config_lexer05.Process(s06);
    bool result06 = config_lexer05.IsSymbol(token06);
    CHECK(result06 == true);

    std::string s07 = "&&";
    emp::Token token07 = config_lexer05.Process(s07);
    bool result07 = config_lexer05.IsSymbol(token07);
    CHECK(result07 == true);

    std::string s08 = "||";
    emp::Token token08 = config_lexer05.Process(s08);
    bool result08 = config_lexer05.IsSymbol(token08);
    CHECK(result08 == true);

    std::string s09 = ">>";
    emp::Token token09 = config_lexer05.Process(s09);
    bool result09 = config_lexer05.IsSymbol(token09);
    CHECK(result09 == true);

    std::string s10 = "<<";
    emp::Token token10 = config_lexer05.Process(s10);
    bool result10 = config_lexer05.IsSymbol(token10);
    CHECK(result10 == true);

    std::string s11 = "<<";
    emp::Token token11 = config_lexer05.Process(s11);
    bool result11 = config_lexer05.IsSymbol(token11);
    CHECK(result11 == true);

    std::string s12 = "<<";
    emp::Token token12 = config_lexer05.Process(s12);
    bool result12 = config_lexer05.IsSymbol(token12);
    CHECK(result12 == true);
  }
}
