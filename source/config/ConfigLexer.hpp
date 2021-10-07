/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  ConfigLexer.hpp
 *  @brief A Lexer used to tokenize MABE config files.
 *  @note Status: BETA
 **/

#ifndef MABE_CONFIG_LEXER_H
#define MABE_CONFIG_LEXER_H

#include "emp/compiler/Lexer.hpp"

namespace mabe {

  class ConfigLexer : public emp::Lexer {
  private:
    int token_identifier = -1;        ///< Token id for identifiers
    int token_number = -1;            ///< Token id for literal numbers
    int token_string = -1;            ///< Token id for literal strings
    int token_char = -1;              ///< Token id for literal characters
    int token_dots = -1;              ///< Token id for a series of dots (...)
    int token_symbol = -1;            ///< Token id for other symbols

  public:
    ConfigLexer() {
      // Whitespace and comments should always be dismissed (top priority)
      IgnoreToken("Whitespace", "[ \t\n\r]+");
      IgnoreToken("//-Comments", "//.*");
      IgnoreToken("/*...*/-Comments", "/[*]([^*]|([*]+[^*/]))*[*]+/");

      // Meaningful tokens have next priority.
      token_identifier = AddToken("Identifier", "[a-zA-Z_][a-zA-Z0-9_]*");
      token_number = AddToken("Literal Number", "[0-9]+(\\.[0-9]+)?");
      token_string = AddToken("Literal String", "\\\"([^\"\\\\]|\\\\.)*\\\"");
      token_char = AddToken("Literal Character", "'([^'\n\\\\]|\\\\.)+'");
      token_dots = AddToken("Dots", "\".\"+");

      /// Symbol tokens should have least priority.  They include any solitary character not listed
      /// above, or pre-specified multi-character groups.
      token_symbol = AddToken("Symbol", ".|\"::\"|\"==\"|\"!=\"|\"<=\"|\">=\"|\"->\"|\"&&\"|\"||\"|\"<<\"|\">>\"|\"++\"|\"--\"|\"**\"");
    }

    bool IsID(const emp::Token token) const noexcept { return token.token_id == token_identifier; }
    bool IsNumber(const emp::Token token) const noexcept { return token.token_id == token_number; }
    bool IsString(const emp::Token token) const noexcept { return token.token_id == token_string; }
    bool IsChar(const emp::Token token) const noexcept { return token.token_id == token_char; }
    bool IsDots(const emp::Token token) const noexcept { return token.token_id == token_dots; }
    bool IsSymbol(const emp::Token token) const noexcept { return token.token_id == token_symbol; }
  };
}

#endif
