/**
 *  @note This file is part of Emplode, currently within https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  Lexer.hpp
 *  @brief A Lexer used to tokenize Emplode config files.
 *  @note Status: BETA
 **/

#ifndef EMPLODE_LEXER_HPP
#define EMPLODE_LEXER_HPP

#include "emp/compiler/Lexer.hpp"

namespace emplode {

  class Lexer : public emp::Lexer {
  private:
    int token_keyword = -1;        ///< Token id for "IF", "WHILE", and other keywords
    int token_identifier = -1;     ///< Token id for identifiers
    int token_number = -1;         ///< Token id for literal numbers
    int token_string = -1;         ///< Token id for literal strings
    int token_dots = -1;           ///< Token id for a series of dots (...)
    int token_symbol = -1;         ///< Token id for other symbols

  public:
    Lexer() {
      // Whitespace and comments should always be dismissed (top priority)
      IgnoreToken("Whitespace", "[ \t\n\r]+");
      IgnoreToken("//-Comments", "//.*");
      IgnoreToken("/*...*/-Comments", "/[*]([^*]|([*]+[^*/]))*[*]+/");

      // Keywords have top priority, especially over identifiers.   Most are simply reserved words.
      token_keyword = AddToken("Keyword",
        "(ELSE)|(IF)"
        // Reserved keywords below.
        "|(AND)|(AUTO)|(BREAK)|(CASE)|(CAST)|(CATCH)|(CLASS)|(CONST)|(CONTINUE)|(DEBUG)"
        "|(DEFAULT)|(DEFINE)|(DELETE)|(DO)|(EVENT)|(EVERY)|(FALSE)|(FOR)|(FOREACH)"
        "|(FUNCTION)|(GOTO)|(IN)|(INCLUDE)|(MUTABLE)|(NAMESPACE)|(NEW)|(OR)|(PRIVATE)"
        "|(PROTECTED)|(PUBLIC)|(RETURN)|(SIGNAL)|(STATIC)|(SWITCH)|(TEMPLATE)|(THIS)"
        "|(THROW)|(TRIGGER)|(TRUE)|(TRY)|(TYPE)|(UNION)|(USING)|(WHILE)|(YIELD)");

      // Meaningful tokens have next priority.
      token_identifier = AddToken("Identifier", "[a-zA-Z_][a-zA-Z0-9_]*");
      token_number = AddToken("Literal Number", "[0-9]+(\\.[0-9]+)?");
      token_string = AddToken("Literal String", "(\\\"([^\"\\\\]|\\\\.)*\\\")|('([^'\\\\]|\\\\.)*')|(`([^`\\\\]|\\\\.)*`)");
      token_dots = AddToken("Dots", "\".\"+");

      /// Symbol tokens should have least priority.  They include any solitary character not listed
      /// above, or pre-specified multi-character groups.
      token_symbol = AddToken("Symbol", ".|\"::\"|\"==\"|\"!=\"|\"<=\"|\">=\"|\"->\"|\"&&\"|\"||\"|\"<<\"|\">>\"|\"++\"|\"--\"|\"**\"");
    }

    bool IsKeyword(const emp::Token token) const noexcept { return token.token_id == token_keyword; }
    bool IsID(const emp::Token token) const noexcept { return token.token_id == token_identifier; }
    bool IsNumber(const emp::Token token) const noexcept { return token.token_id == token_number; }
    bool IsString(const emp::Token token) const noexcept { return token.token_id == token_string; }
    bool IsDots(const emp::Token token) const noexcept { return token.token_id == token_dots; }
    bool IsSymbol(const emp::Token token) const noexcept { return token.token_id == token_symbol; }
  };
}

#endif
