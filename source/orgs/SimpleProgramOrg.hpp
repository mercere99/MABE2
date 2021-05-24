/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file SimpleProgramOrg.hpp
 *  @brief A simple organism with a program-based genome.
 *  @note Status: ALPHA
 */

#ifndef MABE_SIMPLE_PROGRAM_ORGANISM_H
#define MABE_SIMPLE_PROGRAM_ORGANISM_H

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"

#include "emp/base/array.hpp"
#include "emp/base/vector.hpp"
#include "emp/math/Distribution.hpp"
#include "emp/math/random_utils.hpp"

namespace mabe {

  class SimpleProgramOrg : public OrganismTemplate<SimpleProgramOrg> {
  protected:
    enum class Inst {
      INC, DEC,                         // (2) Shift ARG1 by 1
      ADD, SUB, MULT, DIV, MOD,         // (4) Basic two-input math (ARG3 = ARG1 op ARG2)
      NOT, NAND,                        // (2) Boolean logic operations (ARG3 = ARG1 op ARG2)
      SET_REG,                          // (1) Set ARG1 to value determined by ARG2 and ARG3
      MOVE,                             // (1) Remove value from ARG1 and place in ARG2
      COPY,                             // (1) Copy ARG1 into ARG2
      POP,                              // (1) Remove ARG1
      TEST_EQU, TEST_NEQU, TEST_GTR, TEST_LESS, // (4) COMPARE ARG1 and ARG2; put 0/1 result in ARG3
      IF,                               // (1) Inc scope; skip new scope if ARG1 is 0.
      WHILE,                            // (1) Inc scope; repeat as long as ARG1 is non-zero
      COUNTDOWN,                        // (1) Inc scope; repeat and dec ARG1 while non-zero
      CONTINUE,                         // (1) Jump back to WHILE or COUNTDOWN start or dec scope
      BREAK,                            // (1) Jump to end of WHILE or COUNTDOWN scope
      END_SCOPE,                        // (1) Dec scope
      DEFINE, CALL,                     // (2) Inc scope; define creates function, call runs it.
      NUM_BASE_INSTS,                   // 24 - Marker for total instruction count in base set
      ERROR                             // Invalid instruction!
    };

    emp::vector<Inst> genome;           // Series of instructions.
    size_t inst_ptr;                    // Position in genome to execute next.
    emp::vector<size_t> scope_starts;   // Stack of scope starting points.

    // Find the instruction with the provided name.
    Inst GetInst(const std::string & name) const {
      const auto & inst_names = SharedData().inst_names;
      for (size_t i = 0; i < inst_names.size(); i++) {
        if (inst_names[i] == name) return (Inst) i;
      }
      return Inst::ERROR;
    }

    // Find the name associated with a given instruction.
    const std::string & GetName(Inst inst) const {
      return SharedData().inst_names[(size_t) inst];
    }

  public:
    struct ManagerData : public Organism::ManagerData {
      std::string output_name = "vals";  ///< Name of trait that should be used to access values.
      double mut_prob = 0.01;            ///< Probability of position mutating on reproduction.

      // Helper member variables.
      emp::Binomial mut_dist;            ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;          ///< A pre-allocated vector for mutation sites.

      // Instruction Set
      emp::vector<std::string> inst_names;      
    };

    SimpleProgramOrg(OrganismManager<SimpleProgramOrg> & _manager)
      : OrganismTemplate<SimpleProgramOrg>(_manager) { }
    SimpleProgramOrg(const SimpleProgramOrg &) = default;
    SimpleProgramOrg(SimpleProgramOrg &&) = default;
    SimpleProgramOrg(const emp::vector<double> & in, OrganismManager<SimpleProgramOrg> & _manager)
      : OrganismTemplate<SimpleProgramOrg>(_manager)
    {
      // SharedData().ApplyBounds(vals);  // Make sure all data is within range.
    }
    SimpleProgramOrg(size_t N, OrganismManager<SimpleProgramOrg> & _manager)
      : OrganismTemplate<SimpleProgramOrg>(_manager) { }
    ~SimpleProgramOrg() { ; }

    /// Use "to_string" to convert.
    std::string ToString() const override { } // return emp::to_string(vals, ":(TOTAL=", total, ")"); }

    size_t Mutate(emp::Random & random) override {
      // Identify number of and positions for mutations.
      const size_t num_muts = SharedData().mut_dist.PickRandom(random);
      emp::BitVector & mut_sites = SharedData().mut_sites;
      mut_sites.ChooseRandom(random, num_muts);

      // SetVar<double>(SharedData().total_name, total);  // Store total in data map.
      return num_muts;
    }

    void Randomize(emp::Random & random) override {
      for (double & x : vals) {
        x = random.GetDouble(SharedData().min_value, SharedData().max_value);
        total += x;
      }
      SetVar<double>(SharedData().total_name, total);  // Store total in data map.
    }

    /// Put the values in the correct output positions.
    void GenerateOutput() override {
      SetVar<emp::vector<double>>(SharedData().output_name, vals);
      SetVar<double>(SharedData().total_name, total);
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkFuns<size_t>([this](){ return vals.size(); },
                       [this](const size_t & N){ return vals.resize(N, 0.0); },
                       "N", "Number of values in organism");
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each value mutating on reproduction.");
      GetManager().LinkVar(SharedData().mut_size, "mut_size",
                      "Standard deviation on size of mutations.");
      GetManager().LinkVar(SharedData().min_value, "min_value",
                      "Lower limit for value fields.");
      GetManager().LinkVar(SharedData().max_value, "max_value",
                      "Upper limit for value fields.");
      GetManager().LinkMenu(
        SharedData().lower_bound, "lower_bound", "How should the lower limit be enforced?",
        LIMIT_NONE, "no_limit", "Allow values to be arbirarily low.",
        LIMIT_CLAMP, "clamp", "Reduce too-low values to min_value.",
        LIMIT_WRAP, "wrap", "Make low values loop around to maximum.",
        LIMIT_REBOUND, "rebound", "Make low values 'bounce' back up." );
      GetManager().LinkMenu(
        SharedData().upper_bound, "upper_bound", "How should the upper limit be enforced?",
        LIMIT_NONE, "no_limit", "Allow values to be arbirarily high.",
        LIMIT_CLAMP, "clamp", "Reduce too-high values to max_value.",
        LIMIT_WRAP, "wrap", "Make high values loop around to minimum.",
        LIMIT_REBOUND, "rebound", "Make high values 'bounce' back down." );
      GetManager().LinkVar(SharedData().output_name, "output_name",
                      "Name of variable to contain set of values.");
      GetManager().LinkVar(SharedData().total_name, "total_name",
                      "Name of variable to contain total of all values.");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      // Setup the mutation distribution.
      SharedData().mut_dist.Setup(SharedData().mut_prob, vals.size());

      // Setup the default vector to indicate mutation positions.
      SharedData().mut_sites.Resize(vals.size());

      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().output_name,
                                  "Value vector output from organism.",
                                  emp::vector<double>(vals.size()));
      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().total_name,
                                  "Total of all organism outputs.",
                                  0.0);

      // Setup the instruction set.
      SharedData().inst_names.resize((size_t) Inst::NUM_BASE_INSTS);
      SharedData().inst_names[(size_t) Inst::INC] == "Inc";
      SharedData().inst_names[(size_t) Inst::DEC] == "Dec";
      SharedData().inst_names[(size_t) Inst::ADD] == "Add";
      SharedData().inst_names[(size_t) Inst::SUB] == "Sub";
      SharedData().inst_names[(size_t) Inst::MULT] == "Mult";
      SharedData().inst_names[(size_t) Inst::DIV] == "Div";
      SharedData().inst_names[(size_t) Inst::MOD] == "Mod";
      SharedData().inst_names[(size_t) Inst::NOT] == "Not";
      SharedData().inst_names[(size_t) Inst::NAND] == "Nand";
      SharedData().inst_names[(size_t) Inst::SET_REG] == "SetEeg";
      SharedData().inst_names[(size_t) Inst::MOVE] == "Move";
      SharedData().inst_names[(size_t) Inst::COPY] == "Copy";
      SharedData().inst_names[(size_t) Inst::POP] == "Pop";
      SharedData().inst_names[(size_t) Inst::TEST_EQU] == "TestEqu";
      SharedData().inst_names[(size_t) Inst::TEST_NEQU] == "TestNEqu";
      SharedData().inst_names[(size_t) Inst::TEST_GTR] == "TestGtr";
      SharedData().inst_names[(size_t) Inst::TEST_LESS] == "TestLess";
      SharedData().inst_names[(size_t) Inst::IF] == "If";
      SharedData().inst_names[(size_t) Inst::WHILE] == "While";
      SharedData().inst_names[(size_t) Inst::COUNTDOWN] == "Countdown";
      SharedData().inst_names[(size_t) Inst::CONTINUE] == "Continue";
      SharedData().inst_names[(size_t) Inst::BREAK] == "Break";
      SharedData().inst_names[(size_t) Inst::END_SCOPE] == "EndScope";
      SharedData().inst_names[(size_t) Inst::DEFINE] == "Define";
      SharedData().inst_names[(size_t) Inst::CALL] == "Call";
      
    }
  };


  MABE_REGISTER_ORG_TYPE(SimpleProgramOrg, "Organism consisting of a series of N floating-point values.");
}

#endif
