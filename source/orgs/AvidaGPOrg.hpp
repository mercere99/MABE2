/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  AvidaGPOrg.hpp
 *  @brief An organism consisting of lineaer code.
 *  @note Status: ALPHA
 */

#ifndef MABE_AVIDA_GP_ORGANISM_H
#define MABE_AVIDA_GP_ORGANISM_H

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"

#include "emp/datastructs/vector_utils.hpp"
#include "emp/hardware/AvidaGP.hpp"
#include "emp/math/Distribution.hpp"
#include "emp/math/random_utils.hpp"

namespace mabe {

  class AvidaGPOrg : public OrganismTemplate<AvidaGPOrg> {
  protected:
    emp::AvidaGP hardware;

  public:
    AvidaGPOrg(OrganismManager<AvidaGPOrg> & _manager)
      : OrganismTemplate<AvidaGPOrg>(_manager), hardware(GetInstLib()){ }
    AvidaGPOrg(const AvidaGPOrg &) = default;
    AvidaGPOrg(AvidaGPOrg &&) = default;
    ~AvidaGPOrg() { ; }

    struct ManagerData : public Organism::ManagerData {
      // Configuration variables
      double mut_prob = 0.01;              ///< Probability of each bit mutating on reproduction.
      size_t init_length = 100;            ///< Length of new organisms.
      bool init_random = true;             ///< Should we randomize ancestor?  (false = all zeros)
      size_t eval_time = 500;              ///< How long should the CPU be given on each evaluate?
      std::string input_name = "input";    ///< Name of trait that should be used load input values
      std::string output_name = "output";  ///< Name of trait that should be used store output values

      // Internal use
      emp::Binomial mut_dist;            ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;            ///< A pre-allocated vector for mutation sites. 
    };

    /// Use "to_string" to convert.
    std::string ToString() const override { return hardware.ToString(); }

    size_t Mutate(emp::Random & random) override {
      const size_t num_muts = SharedData().mut_dist.PickRandom(random);

      if (num_muts == 0) return 0;
      if (num_muts == 1) {
        const size_t pos = random.GetUInt(hardware.GetSize());
        hardware.RandomizeInst(pos, random);
        return 1;
      }

      // Only remaining option is num_muts > 1.
      auto & mut_sites = SharedData().mut_sites;
      mut_sites.Clear();
      for (size_t i = 0; i < num_muts; i++) {
        const size_t pos = random.GetUInt(hardware.GetSize());
        if (mut_sites[pos]) { --i; continue; }  // Duplicate position; try again.
        hardware.RandomizeInst(pos, random);
      }

      return num_muts;
    }

    void Randomize(emp::Random & random) override {
      for (size_t pos = 0; pos < hardware.GetSize(); pos++) {
        hardware.RandomizeInst(pos, random);
      }
    }

    void Initialize(emp::Random & random) override {
      if (SharedData().init_random) Randomize(random);
    }

    /// Put the output values in the correct output position.
    void GenerateOutput() override {
      hardware.ResetHardware();

      // Setup the input.
      hardware.SetInputs(GetTrait<emp::vector<double>>(SharedData().input_name));

      // Run the code.
      hardware.Process(SharedData().eval_time);

      // Store the results.
      SetTrait<emp::vector<double>>(SharedData().output_name, emp::ToVector(hardware.GetOutputs()));
    }

    static emp::AvidaGP::inst_lib_t& GetInstLib(){
      static emp::AvidaGP::inst_lib_t inst_lib;
      return inst_lib;
    }
    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each instruction mutating on reproduction.");
      GetManager().LinkFuns<size_t>([this](){ return hardware.size(); },
                       [this](const size_t & N){ hardware.Reset(); hardware.PushDefaultInst(N); },
                       "N", "Initial number of instructions in genome");
      GetManager().LinkVar(SharedData().init_random, "init_random",
                      "Should we randomize ancestor?  (0 = \"blank\" default)");
      GetManager().LinkVar(SharedData().eval_time, "eval_time",
                      "How many CPU cycles should we give organisms to run?");
      GetManager().LinkVar(SharedData().input_name, "input_name",
                      "Name of variable to load inputs from.");
      GetManager().LinkVar(SharedData().output_name, "output_name",
                      "Name of variable to output results.");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      // Setup the mutation distribution.
      SharedData().mut_dist.Setup(SharedData().mut_prob, hardware.GetSize());

      // Setup the default vector to indicate mutation positions.
      SharedData().mut_sites.Resize(hardware.GetSize());

      // Setup the input and output traits.
      GetManager().AddRequiredTrait<emp::vector<double>>(SharedData().input_name);
      GetManager().AddSharedTrait(SharedData().output_name,
                                  "Value map output from organism.",
                                  emp::vector<double>());
      SetupInstLib();
    }

    void SetupInstLib(){
      emp::AvidaGP::inst_lib_t& inst_lib = GetInstLib();
      inst_lib.AddInst("Inc", emp::AvidaGP::inst_lib_t::Inst_Inc, 1, "Increment value in reg Arg1");
      inst_lib.AddInst("Dec", emp::AvidaGP::inst_lib_t::Inst_Dec, 1, "Decrement value in reg Arg1");
      inst_lib.AddInst("Not", emp::AvidaGP::inst_lib_t::Inst_Not, 1, "Logically toggle value in reg Arg1");
      inst_lib.AddInst("SetReg", emp::AvidaGP::inst_lib_t::Inst_SetReg, 2, "Set reg Arg1 to numerical value Arg2");
      inst_lib.AddInst("Add", emp::AvidaGP::inst_lib_t::Inst_Add, 3, "regs: Arg3 = Arg1 + Arg2");
      inst_lib.AddInst("Sub", emp::AvidaGP::inst_lib_t::Inst_Sub, 3, "regs: Arg3 = Arg1 - Arg2");
      inst_lib.AddInst("Mult", emp::AvidaGP::inst_lib_t::Inst_Mult, 3, "regs: Arg3 = Arg1 * Arg2");
      inst_lib.AddInst("Div", emp::AvidaGP::inst_lib_t::Inst_Div, 3, "regs: Arg3 = Arg1 / Arg2");
      inst_lib.AddInst("Mod", emp::AvidaGP::inst_lib_t::Inst_Mod, 3, "regs: Arg3 = Arg1 % Arg2");
      inst_lib.AddInst("TestEqu", emp::AvidaGP::inst_lib_t::Inst_TestEqu, 3, "regs: Arg3 = (Arg1 == Arg2)");
      inst_lib.AddInst("TestNEqu", emp::AvidaGP::inst_lib_t::Inst_TestNEqu, 3, "regs: Arg3 = (Arg1 != Arg2)");
      inst_lib.AddInst("TestLess", emp::AvidaGP::inst_lib_t::Inst_TestLess, 3, "regs: Arg3 = (Arg1 < Arg2)");
      inst_lib.AddInst("If", emp::AvidaGP::inst_lib_t::Inst_If, 2, "If reg Arg1 != 0, scope -> Arg2; else skip scope", emp::ScopeType::BASIC, 1);
      inst_lib.AddInst("While", emp::AvidaGP::inst_lib_t::Inst_While, 2, "Until reg Arg1 != 0, repeat scope Arg2; else skip", emp::ScopeType::LOOP, 1);
      inst_lib.AddInst("Countdown", emp::AvidaGP::inst_lib_t::Inst_Countdown, 2, "Countdown reg Arg1 to zero; scope to Arg2", emp::ScopeType::LOOP, 1);
      inst_lib.AddInst("Break", emp::AvidaGP::inst_lib_t::Inst_Break, 1, "Break out of scope Arg1");
      inst_lib.AddInst("Scope", emp::AvidaGP::inst_lib_t::Inst_Scope, 1, "Enter scope Arg1", emp::ScopeType::BASIC, 0);
      inst_lib.AddInst("Define", emp::AvidaGP::inst_lib_t::Inst_Define, 2, "Build function Arg1 in scope Arg2", emp::ScopeType::FUNCTION, 1);
      inst_lib.AddInst("Call", emp::AvidaGP::inst_lib_t::Inst_Call, 1, "Call previously defined function Arg1");
      inst_lib.AddInst("Push", emp::AvidaGP::inst_lib_t::Inst_Push, 2, "Push reg Arg1 onto stack Arg2");
      inst_lib.AddInst("Pop", emp::AvidaGP::inst_lib_t::Inst_Pop, 2, "Pop stack Arg1 into reg Arg2");
      inst_lib.AddInst("Input", emp::AvidaGP::inst_lib_t::Inst_Input, 2, "Pull next value from input Arg1 into reg Arg2");
      inst_lib.AddInst("Output", emp::AvidaGP::inst_lib_t::Inst_Output, 2, "Push reg Arg1 into output Arg2");
      inst_lib.AddInst("CopyVal", emp::AvidaGP::inst_lib_t::Inst_CopyVal, 2, "Copy reg Arg1 into reg Arg2");
      inst_lib.AddInst("ScopeReg", emp::AvidaGP::inst_lib_t::Inst_ScopeReg, 1, "Backup reg Arg1; restore at end of scope");

      for (size_t i = 0; i < emp::AvidaGP::CPU_SIZE; i++) {
        GetInstLib().AddArg(emp::to_string((int)i), i);                   // Args can be called by value
        GetInstLib().AddArg(emp::to_string("Reg", 'A'+(char)i), i);  // ...or as a register.
      }
      LoadExternalInstructions();
    }

    void LoadExternalInstructions(){
      emp::AvidaGP::inst_lib_t& inst_lib = GetInstLib();
      ActionMap& action_map = GetManager().GetControl().GetActionMap(0);
      emp::vector<mabe::Action>& action_vec = action_map.GetFuncs<void,emp::AvidaGP&>();
      std::cout << "Found " << action_vec.size() << " external functions!" << std::endl;
      for(size_t action_idx = 0; action_idx < action_vec.size(); ++action_idx){
        emp::AnyFunction& func = action_vec[action_idx].function;
        inst_lib.AddInst(
            action_vec[action_idx].name,
            [&func](emp::AvidaGP& org, const emp::AvidaGP::inst_t& inst){
              func.Call<void, emp::AvidaGP&>(org);
            },
            0, 
            "test");
        std::cout << "Added instruction: " << action_vec[action_idx].name << std::endl;
      }
    }

  };

  MABE_REGISTER_ORG_TYPE(AvidaGPOrg, "Organism consisting of Avida instructions.");
}

#endif
