/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  MABEScript.hpp
 *  @brief Customized Emplode scripting language instance for MABE runs.
 */

#ifndef MABE_MABE_SCRIPT_HPP
#define MABE_MABE_SCRIPT_HPP

#include <limits>
#include <string>
#include <sstream>

#include "emp/base/array.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/data/DataMap.hpp"
#include "emp/data/DataMapParser.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/string_utils.hpp"

#include "../Emplode/Emplode.hpp"

#include "Collection.hpp"
#include "data_collect.hpp"
#include "MABEBase.hpp"
#include "ModuleBase.hpp"
#include "Population.hpp"
#include "SigListener.hpp"
#include "TraitManager.hpp"

namespace mabe {

  ///  @brief The MABE scripting language.

  class MABEScript : public emplode::Emplode {
  private:
    MABEBase & control;
    emp::DataMapParser dm_parser;       ///< Parser to process functions on a data map

    using Symbol_Var = emplode::Symbol_Var;

    struct PreprocessResults {
      std::string result;         // Updated string
      emp::vector<double> values; // Numerical values kept aside, if preserve_nums=true;
    };

  public:
    /// Build a function to scan a data map, run a provided equation on its entries,
    /// and return the result.
    auto BuildTraitEquation(const emp::DataLayout & data_layout, std::string equation) {
      auto pp_equ = Preprocess(equation, true);
      auto dm_fun = dm_parser.BuildMathFunction(data_layout, pp_equ.result, pp_equ.values);
      return [dm_fun](const Organism & org){ return dm_fun(org.GetDataMap()); };
    }

    /// Scan an equation and return the names of all traits it is using.
    const std::set<std::string> & GetEquationTraits(const std::string & equation) {
      return dm_parser.GetNamesUsed(equation);
    }

    /// Find any instances of ${X} and eval the X.
    PreprocessResults Preprocess(const std::string & in_string, bool preserve_nums=false) {
      PreprocessResults pp_out;
      pp_out.result = in_string;

      // Seek out instances of "${" to indicate the start of pre-processing.
      for (size_t i = 0; i < pp_out.result.size(); ++i) {
        if (pp_out.result[i] != '$') continue;   // Replacement tag must start with a '$'.
        if (pp_out.result.size() <= i+2) break;  // Not enough room for a replacement tag.
        if (pp_out.result[i+1] == '$') {         // Compress two $$ into one $
          pp_out.result.erase(i,1);
          continue;
        }
        if (pp_out.result[i+1] != '{') continue; // Eval must be surrounded by braces.

        // If we made it this far, we have a starting match!
        size_t end_pos = emp::find_paren_match(pp_out.result, i+1, '{', '}', false);
        if (end_pos == i+1) {
          emp::notify::Warning("In pre-processing:\n  '", in_string,
                               "',\nfound '${' with no matching '}'.");
          return pp_out; // Stop where we are... No end brace found!
        }
        emp::Datum replacement = Execute(emp::view_string_range(pp_out.result, i+2, end_pos));
        // Test if we should drop the replacement results directly in-line.
        if (!preserve_nums || replacement.IsString()) {
          std::string new_str = replacement.AsString();   // Get new text.
          pp_out.result.replace(i, end_pos-i+1, new_str); // Put into place.
          i += new_str.size();                            // Continue at the next position...
        }
        else { // Replacement is numerical and needs to be preserved...
          std::string new_str = emp::to_string("$",pp_out.values.size()); // Generate the '$#'
          pp_out.result.replace(i, end_pos-i+1, new_str);                 // Put it in place.
          pp_out.values.push_back(replacement.NativeDouble());            // Store associated value
          i += new_str.size();                                            // Find continue pos
        }
      }

      return pp_out;
    }


    /// Build a function to scan a collection of organisms, calculating a given trait_fun for each,
    /// aggregating those values based on the mode, and returning the result as the specifed type.
    ///
    ///  'mode' option are:
    ///   <none>      : Default to the value of the trait for the first organism in the collection.
    ///   [ID]        : Value of this trait for the organism at the given index of the collection.
    ///   [OP][VALUE] : Count how often this value has the [OP] relationship with [VALUE].
    ///                  [OP] can be ==, !=, <, >, <=, or >=
    ///                  [VALUE] can be any numeric value
    ///   [OP][TRAIT] : Count how often this trait has the [OP] relationship with [TRAIT]
    ///                  [OP] can be ==, !=, <, >, <=, or >=
    ///                  [TRAIT] can be any other trait name
    ///   unique      : Return the number of distinct value for this trait (alias="richness").
    ///   mode        : Return the most common value in this collection (aliases="dom","dominant").
    ///   min         : Return the smallest value of this trait present.
    ///   max         : Return the largest value of this trait present.
    ///   ave         : Return the average value of this trait (alias="mean").
    ///   median      : Return the median value of this trait.
    ///   variance    : Return the variance of this trait.
    ///   stddev      : Return the standard deviation of this trait.
    ///   sum         : Return the summation of all values of this trait (alias="total")
    ///   entropy     : Return the Shannon entropy of this value.
    ///   :trait      : Return the mutual information with another provided trait.

    template <typename FROM_T=Collection>
    std::function<Symbol_Var(const FROM_T &)> BuildTraitSummary(
      std::string trait_fun,         // Function to calculate on each organism
      std::string summary_type,      // Method to combine organism results ("max", "mean", etc.)
      emp::DataLayout & data_layout  // DataLayout to assume for this summary
    ) {
      static_assert( std::is_same<FROM_T,Collection>() ||  std::is_same<FROM_T,Population>(),
                    "BuildTraitSummary FROM_T must be Collection or Population." );

      // Pre-process the trait function to allow for use of regular config variables.
      trait_fun = Preprocess(trait_fun).result;

      // The trait input has two components:
      // (1) the trait (or trait function) and
      // (2) how to calculate the trait SUMMARY, such as min, max, ave, etc.

      // If we have a single trait, we may want to use a string type.
      if (emp::is_identifier(trait_fun)           // If we have a single trait...
          && data_layout.HasName(trait_fun)      // ...and it's in the data map...
          && !data_layout.IsNumeric(trait_fun)   // ...and it's not numeric...
      ) {
        size_t trait_id = data_layout.GetID(trait_fun);
        emp::TypeID result_type = data_layout.GetType(trait_id);

        auto get_fun = [trait_id, result_type](const Organism & org) {
          return emp::to_literal( org.GetTraitAsString(trait_id, result_type) );
        };
        auto fun = BuildCollectFun<std::string, Collection>(summary_type, get_fun);

        // If we are coming from a Population, first convert to a collection.
        if constexpr (std::is_same<FROM_T,Population>()) {
          return [fun](const Population & p){ return fun( Collection(p) ); };
        }
        else {
          return fun;
        }
      }

      // If we made it here, we are numeric.
      auto get_fun = BuildTraitEquation(data_layout, trait_fun);
      auto fun = BuildCollectFun<double, Collection>(summary_type, get_fun);

      // If we don't have a fun, we weren't able to build an aggregation function.
      if (!fun) {
        emp::notify::Error("Unknown trait filter '", summary_type, "' for trait '", trait_fun, "'.");
        return [](const FROM_T &){ return Symbol_Var(0); };
      }

      // If we are processing a Population, first convert it to a Collection.
      if constexpr (std::is_same<FROM_T,Population>()) {
        return [fun](const Population & p){ return fun( Collection(p) ); };
      }

      return fun;
    }


    /// Build a function that takes a trait equation, builds it, and runs it on a container.
    /// Output is a function in the form:  TO_T(const FROM_T &, string equation, TO_T default)
    template <typename FROM_T=Collection> 
    auto BuildTraitFunction(const std::string & fun_type) {
      return [this,fun_type](FROM_T & group, const std::string & equation) {
        auto trait_fun = BuildTraitSummary<FROM_T>(equation, fun_type, group.GetDataLayout());
        return trait_fun(group);
      };
    }

    template <typename GROUP_T>
    void Initialize_GroupType(emplode::TypeInfo & type_info) {
      type_info.AddMemberFunction("TRAIT", BuildTraitFunction<GROUP_T>("0"),
        "Return the value of the provided trait for the first organism");
      type_info.AddMemberFunction("CALC_RICHNESS", BuildTraitFunction<GROUP_T>("richness"),
        "Count the number of distinct values of a trait (or equation).");
      type_info.AddMemberFunction("CALC_MODE", BuildTraitFunction<GROUP_T>("mode"),
        "Identify the most common value of a trait (or equation).");
      type_info.AddMemberFunction("CALC_MEAN", BuildTraitFunction<GROUP_T>("mean"),
        "Calculate the average value of a trait (or equation).");
      type_info.AddMemberFunction("CALC_MIN", BuildTraitFunction<GROUP_T>("min"),
        "Find the smallest value of a trait (or equation).");
      type_info.AddMemberFunction("CALC_MAX", BuildTraitFunction<GROUP_T>("max"),
        "Find the largest value of a trait (or equation).");
      type_info.AddMemberFunction("ID_MIN", BuildTraitFunction<GROUP_T>("min_id"),
        "Find the index of the smallest value of a trait (or equation).");
      type_info.AddMemberFunction("ID_MAX", BuildTraitFunction<GROUP_T>("max_id"),
        "Find the index of the largest value of a trait (or equation).");
      type_info.AddMemberFunction("CALC_MEDIAN", BuildTraitFunction<GROUP_T>("median"),
        "Find the 50-percentile value of a trait (or equation).");
      type_info.AddMemberFunction("CALC_VARIANCE", BuildTraitFunction<GROUP_T>("variance"),
        "Find the variance of the distribution of values of a trait (or equation).");
      type_info.AddMemberFunction("CALC_STDDEV", BuildTraitFunction<GROUP_T>("stddev"),
        "Find the variance of the distribution of values of a trait (or equation).");
      type_info.AddMemberFunction("CALC_SUM", BuildTraitFunction<GROUP_T>("sum"),
        "Add up the total value of a trait (or equation).");
      type_info.AddMemberFunction("CALC_ENTROPY", BuildTraitFunction<GROUP_T>("entropy"),
        "Determine the entropy of values for a trait (or equation).");

      type_info.AddMemberFunction("FIND_MIN",
        [this](GROUP_T & group, const std::string & trait_equation) -> Collection {
          if (group.IsEmpty()) Collection{};
          auto trait_fun =
            BuildTraitSummary<GROUP_T>(trait_equation, "min_id", group.GetDataLayout());
          return group.IteratorAt(trait_fun(group)).AsPosition();
        },
        "Produce OrgList with just the org with the minimum value of the provided function.");
      type_info.AddMemberFunction("FIND_MAX",
        [this](GROUP_T & group, const std::string & trait_equation) -> Collection {
          if (group.IsEmpty()) Collection{};
          auto trait_fun =
            BuildTraitSummary<GROUP_T>(trait_equation, "max_id", group.GetDataLayout());
          return group.IteratorAt(trait_fun(group)).AsPosition();
        },
        "Produce OrgList with just the org with the maximum value of the provided function.");
    }

  private:
    /// ======= Helper functions ===

    /// Set up all of the functions and globals in MABEScript
    void Initialize() {
      // Setup main MABE variables.
      auto & root_scope = GetSymbolTable().GetRootScope();
      root_scope.LinkFuns<int>("random_seed",
                              [this](){ return control.GetRandomSeed(); },
                              [this](int seed){ control.SetRandomSeed(seed); },
                              "Seed for random number generator; use 0 to base on time.");

      // Setup "Population" as a type in the config file.
      auto pop_init_fun = [this](const std::string & name) { return &control.AddPopulation(name); };
      auto pop_copy_fun = [this](const EmplodeType & from, EmplodeType & to) {
        emp::Ptr<const Population> from_pop = dynamic_cast<const Population *>(&from);
        emp::Ptr<Population> to_pop = dynamic_cast<Population *>(&to);
        if (!from_pop || !to_pop) return false; // Wrong type!
        control.CopyPop(*from_pop, *to_pop);    // Do the actual copy.
        return true;
      };
      emplode::TypeInfo & pop_type = AddType<Population>("Population", "Collection of organisms",
                                             pop_init_fun, pop_copy_fun);

      // Setup "Collection" as another config type.
      emplode::TypeInfo & collect_type = AddType<Collection>("OrgList", "Collection of organism pointers");

      Initialize_GroupType<Population>(pop_type);
      Initialize_GroupType<Collection>(collect_type);

      pop_type.AddMemberFunction("REPLACE_WITH",
        [this](Population & to_pop, Population & from_pop){
          control.MoveOrgs(from_pop, to_pop, true); return 0;
        }, "Move all organisms organisms from another population, removing current orgs." );
      pop_type.AddMemberFunction("APPEND",
        [this](Population & to_pop, Population & from_pop){
          control.MoveOrgs(from_pop, to_pop, false); return 0;
        }, "Move all organisms organisms from another population, adding after current orgs." );

      pop_type.AddMemberFunction("FILTER",
        [this](Population & pop, const std::string & trait_equation) -> Collection {
          Collection out_collect;
          if (pop.GetNumOrgs() > 0) { // Only do this work if we actually have organisms!
            auto filter = BuildTraitEquation(pop.GetDataLayout(), trait_equation);
            for (auto it = pop.begin(); it != pop.end(); ++it) {
              if (filter(*it)) out_collect.Insert(it);
            }
          }
          return out_collect;
        },
        "Produce OrgList with just the orgs that pass through the filter criteria.");

      // ------ DEPRECATED FUNCTION NAMES ------
      Deprecate("EVAL", "EXEC");
      Deprecate("exit", "EXIT");
      Deprecate("inject", "INJECT");
      Deprecate("print", "PRINT");

      // Add other built-in functions to the config file.
      AddFunction("EXIT", [this](){ control.RequestExit(); return 0; }, "Exit from this MABE run.");
      AddFunction("GET_UPDATE", [this](){ return control.GetUpdate(); }, "Get current update.");
      AddFunction("GET_VERBOSE", [this](){ return control.GetVerbose(); }, "Has the verbose flag been set?");

      std::function<std::string(const std::string &)> preprocess_fun =
        [this](const std::string & str) { return Preprocess(str).result; };
      AddFunction("PP", preprocess_fun, "Preprocess a string (replacing any ${...} with result.)");

      // Add in built-in event triggers; these are used to indicate when events should happen.
      AddSignal("START");   // Triggered at the beginning of a run.
      AddSignal("UPDATE");  // Tested every update.
    }


    void Deprecate(const std::string & old_name, const std::string & new_name) {
      auto dep_fun = [this,old_name,new_name](const emp::vector<emp::Ptr<emplode::Symbol>> &){
          std::cerr << "Function '" << old_name << "' deprecated; use '" << new_name << "'\n";
          control.RequestExit();
          return 0;      
        };

      AddFunction(old_name, dep_fun, std::string("Deprecated.  Use: ") + new_name);
    }
    
  public:
    MABEScript(MABEBase & in) : control(in), dm_parser(true, in.GetRandom()) { Initialize(); }
    ~MABEScript() { }

  };

}

#endif
