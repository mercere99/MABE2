/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019
 *
 *  @file  Config.h
 *  @brief Manages all configuration of MABE runs.
 *  @note Status: ALPHA
 * 
 *  Example usage:
 *   a = 7;              // a is a variable with the value 7
 *   b = "balloons";     // b is a variable equal to the literal string "balloons"
 *   c = a + 10;         // '+' will add values; c is a variable equal to 17.
 *   d = "99 " + b;      // '+' will append strings; d is a variable equal to "99 balloons"
 *   // e = "abc" + 123; // ERROR - cannot add strings and values!
 *   f = {               // f is a structure/scope/dictionary
 *     g = 1;
 *     h = "two";
 *     i = {
 *       j = 3;
 *     }
 *     a = "shadow!";    // A variable can be redeclared in other scopes, shadowing the original.
 *     j = "spooky!";    // A NEW variable since we are out of the namespace of the other j.
 *     b = i.j;          // Namespaces can be stepped through with dots.
 *     c = .b;           // A variable name beginning with a dot indicates global namespace.
 *   }                   // f has been initialized with seven variables in its scope.
 *   f["new"] = 22;      // You can always add new fields to structures.
 *   k = [ 1 , 2 , 3];   // k is a vector of values (vectors must have all types the same!)
 *   l = k[1];           // Vectors can be indexed into.
 *   m() = a * c;        // Functions have parens after the variable name; evaluated when called.
 *   n(o,p) = o + p;     // Functions may have arguments.
 *   q = 'q';            // Literal chars are translated immediately to their ascii value
 * 
 *   // There are a few pre-defined functions at the global namespace.
 *   r = size(k);        // = 3  (always a value)
 *   s = names(f);       // = ["a","b","c","g","h","i","j"] (vector of strings in alphabetical order)
 *   t = string(c);      // = "17"  (convert value to string)
 *   u = value(t+"00");  // = 1700  (convert string to value)
 * 
 * 
 *  In practice:
 *   organism_types = {
 *     Sheep = {
 *       class = MarkovBrain;
 *       outputs = 10;
 *       node_weights = 0.75;
 *       recurrance = 5;
 *     }
 *     Wolves = {
 *       class = MarkovBrain;
 *       outputs = 10;
 *       node_weights = 0.75;
 *       recurrance = 3;
 *     }
 *   }
 *   modules = {
 *     Mutations = {
 *       copy_prob = 0.001;
 *       insert_prob = 0.05;
 *     }
 *   }
 */

#ifndef MABE_CONFIG_H
#define MABE_CONFIG_H

#include "base/assert.h"
#include "meta/TypeID.h"
#include "tools/string_utils.h"

#include "ConfigEntry.h"
#include "ConfigLink.h"

namespace mabe {

  class Config {
  protected:

  public:

  };

}
#endif
