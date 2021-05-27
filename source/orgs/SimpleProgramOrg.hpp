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

#include <math.h>

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
    using base_t = OrganismTemplate<SimpleProgramOrg>;
    using base_t::SharedData;

    enum class Inst {
      GET_CONST, ADD_CONST, SHIFT_CONST, // (3) Modify ARG1 by ARG2c
      ADD, SUB, MULT, DIV, MOD,          // (5) Basic two-input math (ARG3 = ARG1 op ARG2)
      NOT, AND, OR,                      // (3) Bitwise logic operations (ARG3 = ARG1 op ARG2)
      COPY,                              // (1) Copy ARG1 into ARG2
      TEST_EQU, TEST_NEQU, TEST_GTR, TEST_LESS, // (4) COMPARE ARG1 and ARG2; put 0/1 result in ARG3
      IF,                                // (1) Inc scope; skip new scope if ARG1 is 0.
      WHILE,                             // (1) Inc scope; repeat as long as ARG1 is non-zero
      COUNTDOWN,                         // (1) Inc scope; repeat and dec ARG1 while non-zero
      CONTINUE,                          // (1) Jump back to WHILE or COUNTDOWN start or end scope
      BREAK,                             // (1) Jump to end of WHILE or COUNTDOWN scope
      END_SCOPE,                         // (1) Dec scope
      PUSH, POP,                         // (2) Treat ARG1 as stack pointer; push/pop with ARG2
      NUM_BASE_INSTS,                    // 26 - Marker for total instruction count in base set
      NONE,                              // Empty instruction!
      ERROR                              // Invalid instruction!
    };

    // Arguments can be values (constants) or variables (direct or indirect memory positions).
    // Constants are just used in CONST instrustions where ARG2c is a direct value centered at zero
    //   Range is -11 to 12 by default.
    // Variables are:
    //   direct registers (10: A-J), first four are regs only; next 6 are ALSO indirect memory.
    //   indirections to internal memory (2: E,F),
    //   indirections to input memory (2: G,H), uses offset of + 512.
    //   indirections to output memory (2: I,J), uses offset of + 768.

    static constexpr size_t NUM_REGS = 16;
    static constexpr size_t MEM_SIZE = 1024;
    static constexpr size_t MEM_IO_SIZE = 256;
    static constexpr size_t MEM_INTERNAL_START = 0;
    static constexpr size_t MEM_INPUT_START = 512;
    static constexpr size_t MEM_OUTPUT_START = MEM_INPUT_START + MEM_IO_SIZE;
    static_assert(MEM_OUTPUT_START + MEM_IO_SIZE <= MEM_SIZE, "IO must fit inside other memoery.");

    static constexpr size_t MEM_MASK = MEM_SIZE - 1;
    static constexpr size_t REG_MASK = NUM_REGS - 1;

    using genome_t = emp::vector<unsigned char>;
    using memory_t = emp::array<double, MEM_SIZE>;

    genome_t genome;                    // Series of instructions.
    size_t inst_ptr;                    // Position in genome to execute next.
    memory_t mem;   

    emp::vector<size_t> scope_starts;   // Stack of scope starting points.
    emp::vector<size_t> fun_starts;     // Positions where specific functions begin.
    emp::vector<size_t> call_stack;     // Where should function ends return to?

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

    // Convert an argument to the associated variable.
    double & GetArgVar(const unsigned char arg) {
      // We're assuming 16 registers, where the last 6 are indirections.
      switch (arg & REG_MASK) {
        case 0: return mem[0];
        case 1: return mem[1];
        case 2: return mem[2];
        case 3: return mem[3];
        case 4: return mem[4];
        case 5: return mem[5];
        case 6: return mem[6];
        case 7: return mem[7];
        case 8: return mem[8];
        case 9: return mem[9];
        case 10: return mem[(((size_t) mem[4]) & MEM_MASK)]; // Internal memory.
        case 11: return mem[(((size_t) mem[5]) & MEM_MASK)]; // Internal memory.
        case 12: return mem[((MEM_INPUT_START + (size_t) mem[6]) & MEM_MASK)]; // Input memory.
        case 13: return mem[((MEM_INPUT_START + (size_t) mem[7]) & MEM_MASK)]; // Input memory.
        case 14: return mem[((MEM_OUTPUT_START + (size_t) mem[8]) & MEM_MASK)]; // Output memory.
        case 15: return mem[((MEM_OUTPUT_START + (size_t) mem[9]) & MEM_MASK)]; // Output memory.
      };
    }

    // Convert an argument variable to a size_t
    size_t GetArgBits(const unsigned char arg) {
      return (size_t) GetArgVar(arg);
    }

    // Convert an argument to the associated constant.
    double GetArgConst(const unsigned char arg) {
      // Easy access to a range of potentially useful constants.
      switch (arg & REG_MASK) {
        case 0:  return  -2.0;
        case 1:  return  -1.0;
        case 2:  return   0.0;
        case 3:  return   0.0;  // Second zero since it's the most useful value.
        case 4:  return   0.25;
        case 5:  return   0.5;
        case 6:  return   1.0;
        case 7:  return   2.0;
        case 8:  return   3.0;
        case 9:  return   4.0;
        case 10: return   8.0;
        case 11: return  16.0;
        case 12: return  32.0;
        case 13: return  64.0;
        case 14: return 128.0;
        case 15: return 256.0;
      };
    }

    // What kind of scope are we in?
    Inst GetScopeType() {
      if (scope_starts.size() == 0) return Inst::NONE;
      switch (genome[scope_starts.back()]) {
        case (size_t) Inst::IF:        return Inst::IF;
        case (size_t) Inst::WHILE:     return Inst::WHILE;
        case (size_t) Inst::COUNTDOWN: return Inst::COUNTDOWN;

        default: return Inst::ERROR; // The above are the only legal scope types!
      };
    }

    // Jump past the current scope.
    void SkipScope() {
      int scope_level = 1;
      while (scope_level > 0 && inst_ptr < genome.size()) {
        switch (genome[inst_ptr]) {
          case (size_t) Inst::IF:
          case (size_t) Inst::WHILE:
          case (size_t) Inst::COUNTDOWN:
            scope_level++;
            break;
          case (size_t) Inst::END_SCOPE:
            scope_level--;
        }
      }
    }

    // Execute the next instruction.
    void RunInst() {
      // Loop around to zero if we're off the end.
      if (inst_ptr >= genome.size()) { inst_ptr = 0; }

      const unsigned char cur_inst = genome[inst_ptr];
      const unsigned char arg1 = genome[inst_ptr+1];
      const unsigned char arg2 = genome[inst_ptr+2];
      const unsigned char arg3 = genome[inst_ptr+3];
      inst_ptr += 4;

      if (cur_inst < (unsigned char) Inst::NUM_BASE_INSTS) {
        switch ((Inst) cur_inst) {
        case Inst::GET_CONST:       // Set ARG1 to the constant value represented by ARG2
          GetArgVar(arg1) = GetArgConst(arg2);
          break;
        case Inst::ADD_CONST:
          GetArgVar(arg1) += GetArgConst(arg2);
          break;
        case Inst::SHIFT_CONST:
          GetArgVar(arg1) *= emp::Pow2(GetArgConst(arg2));
          break;
        case Inst::ADD:
          GetArgVar(arg3) = GetArgVar(arg1) + GetArgVar(arg2);
          break;
        case Inst::SUB:
          GetArgVar(arg3) = GetArgVar(arg1) - GetArgVar(arg2);
          break;
        case Inst::MULT:
          GetArgVar(arg3) = GetArgVar(arg1) * GetArgVar(arg2);
          break;
        case Inst::DIV:
          if (GetArgVar(arg2) != 0.0) GetArgVar(arg3) = GetArgVar(arg1) / GetArgVar(arg2);
          // @CAO Do something on error?
          break;
        case Inst::MOD:
          if (GetArgVar(arg2) != 0.0) GetArgVar(arg3) = std::remainder(GetArgVar(arg1), GetArgVar(arg2));
          // @CAO Do something on error?
          break;
        case Inst::NOT:
          GetArgVar(arg3) = ~GetArgBits(arg1);
          break;
        case Inst::AND:
          GetArgVar(arg3) = GetArgBits(arg1) & GetArgBits(arg2);
          break;
        case Inst::OR:
          GetArgVar(arg3) = GetArgBits(arg1) | GetArgBits(arg2);
          break;
        case Inst::COPY:
          GetArgVar(arg2) = GetArgVar(arg1);
          break;
        case Inst::TEST_EQU:
          GetArgVar(arg3) = (GetArgVar(arg1) == GetArgVar(arg2));
          break;
        case Inst::TEST_NEQU:
          GetArgVar(arg3) = (GetArgVar(arg1) != GetArgVar(arg2));
          break;
        case Inst::TEST_GTR:
          GetArgVar(arg3) = (GetArgVar(arg1) > GetArgVar(arg2));
          break;
        case Inst::TEST_LESS:
          GetArgVar(arg3) = (GetArgVar(arg1) < GetArgVar(arg2));
          break;

        case Inst::IF:
        case Inst::WHILE:
        case Inst::COUNTDOWN:  // Differ only at END_SCOPE
          scope_starts.push_back(inst_ptr - 4); // Enter a new scope!
          if (GetArgBits(arg1) == 0) SkipScope();
          break;

        case Inst::CONTINUE:  // Return to the begining of this scope!
          // Skip over any 'IF' scopes that we may be in.
          while (GetScopeType() == Inst::IF) scope_starts.pop_back();

          // If we are in a loop, go back to the start; otherwise go to the start of the genome.
          switch (GetScopeType()) {
            case Inst::NONE: inst_ptr = 0; break;
            case Inst::COUNTDOWN:
              GetArgVar(arg1) -= 1.0;
              [[fallthrough]];
            case Inst::WHILE:
              inst_ptr = scope_starts.back();
              break;
            default:
              emp_error("Internal error; Invalid context for CONTINUE");
          };

          break;
        case Inst::BREAK:
          // Skip over any 'IF' scopes that we may be in, and then one more for break.
          while (GetScopeType() == Inst::IF) SkipScope();
          SkipScope();
          break;
        case Inst::END_SCOPE:
          switch (GetScopeType()) {
            case Inst::NONE: break;                         // No scope?  Ignore it!
            case Inst::IF: scope_starts.pop_back(); break;  // We are done with the IF!
            case Inst::COUNTDOWN:
              GetArgVar(arg1) -= 1.0;
              [[fallthrough]];
            case Inst::WHILE:
              inst_ptr = scope_starts.back();
              break;
            default:
              emp_error("Internal error; Invalid context for CONTINUE");
          };          
          break;
        case Inst::PUSH:
          break;
        case Inst::POP:
          break;
        };
      }
      // Special instruction! (or nop?)
      else {

      }
    }

  public:
    struct ManagerData : public Organism::ManagerData {
      std::string output_name = "vals";  ///< Name of trait that should be used to access values.
      double mut_prob = 0.01;            ///< Probability of position mutating on reproduction.

      // Helper member variables.
      emp::Binomial mut_dist;            ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;          ///< A pre-allocated vector for mutation sites.

      // Instruction Set
      emp::vector<std::string> inst_names;  ///< Names of all instructions in use.
      size_t num_regs;                      ///< Number of registers in the CPU.
      size_t num_indirect_args;             ///< Number of indirect args for each group.
      size_t const_shift;                   ///< How far should constant arguments be shifted?
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
      auto & data = SharedData();

      // Setup the mutation distribution.
      data.mut_dist.Setup(data.mut_prob, vals.size());

      // Setup the default vector to indicate mutation positions.
      data.mut_sites.Resize(vals.size());

      // Setup the output trait.
      GetManager().AddSharedTrait(data.output_name,
                                  "Value vector output from organism.",
                                  emp::vector<double>(vals.size()));
      // Setup the output trait.
      GetManager().AddSharedTrait(data.total_name,
                                  "Total of all organism outputs.",
                                  0.0);

      // Setup the instruction set.
      data.inst_names.resize((size_t) Inst::NUM_BASE_INSTS);
      data.inst_names[(size_t) Inst::GET_CONST] = "GetConst":
      data.inst_names[(size_t) Inst::ADD_CONST] = "AddConst":
      data.inst_names[(size_t) Inst::SHIFT_CONST] = "ShiftConst":
      data.inst_names[(size_t) Inst::ADD] == "Add";
      data.inst_names[(size_t) Inst::SUB] == "Sub";
      data.inst_names[(size_t) Inst::MULT] == "Mult";
      data.inst_names[(size_t) Inst::DIV] == "Div";
      data.inst_names[(size_t) Inst::MOD] == "Mod";
      data.inst_names[(size_t) Inst::NOT] == "Not";
      data.inst_names[(size_t) Inst::AND] == "And";
      data.inst_names[(size_t) Inst::OR] == "Or";
      data.inst_names[(size_t) Inst::COPY] == "Copy";
      data.inst_names[(size_t) Inst::TEST_EQU] == "TestEqu";
      data.inst_names[(size_t) Inst::TEST_NEQU] == "TestNEqu";
      data.inst_names[(size_t) Inst::TEST_GTR] == "TestGtr";
      data.inst_names[(size_t) Inst::TEST_LESS] == "TestLess";
      data.inst_names[(size_t) Inst::IF] == "If";
      data.inst_names[(size_t) Inst::WHILE] == "While";
      data.inst_names[(size_t) Inst::COUNTDOWN] == "Countdown";
      data.inst_names[(size_t) Inst::CONTINUE] == "Continue";
      data.inst_names[(size_t) Inst::BREAK] == "Break";
      data.inst_names[(size_t) Inst::END_SCOPE] == "EndScope";
      data.inst_names[(size_t) Inst::POP] == "Pop";
      data.inst_names[(size_t) Inst::PUSH] == "Push";
      
      // @CAO ADD NON-STANDARD INSTRUCTIONS!

      data.num_indirect_args = data.inst_names.size() / 8;
      data.const_shift = 4 * data.num_indirect_args;
      data.num_regs = data.inst_names.size() - data.const_shift;
    }
  };


  MABE_REGISTER_ORG_TYPE(SimpleProgramOrg, "Organism consisting of a series of N floating-point values.");
}

#endif
