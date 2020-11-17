/**
*  @note This file is part of MABE, https://github.com/mercere99/MABE2
*  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
*  @date 2019-2020.
*
*  @file  AvidaGP.h
*  @brief Genetic programming focused Avida organism. 
*  @note Status: ALPHA
*/

#ifndef MABE_AVIDA_GP_ORGANISM_H
#define MABE_AVIDA_GP_ORGANISM_H

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"
//#include "./AvidaGP_InstLib.hpp"

#include "emp/math/Distribution.hpp"
#include "emp/math/random_utils.hpp"
#include "emp/hardware/AvidaGP.hpp"

namespace mabe {

  class AvidaGPOrg : public OrganismTemplate<AvidaGPOrg>, public emp::AvidaCPU_Base<AvidaGPOrg> {
  public:

    using base_t = AvidaCPU_Base<AvidaGPOrg>;
    using typename base_t::genome_t;
    using typename base_t::inst_t;
    using typename base_t::inst_lib_t;
    
    // TODO: Shift birth logic to mirror that of Avida
    static void Inst_StartBirth(hardware_t & hw, const inst_t & inst) {
      hw.IncrementBirthCounter();
    }
    
    /// Modified verion of inst_lib_t::Inst_Output that triggers evaluation
    static void Inst_Output_Trig(hardware_t & hw, const inst_t & inst);

    /// Performs bitwise NAND of the two specified registers and stores result in a specified register
    static void Inst_Nand(hardware_t & hw, const inst_t & inst) {
      const uint64_t a = static_cast<uint64_t>(hw.regs[inst.args[0]]);
      const uint64_t b = static_cast<uint64_t>(hw.regs[inst.args[1]]);
      hw.regs[inst.args[2]] = static_cast<double>(~(a & b));
    }

    // TODO: Allow instruction library to be modified in config
    /// Singleton that returns a modified instruction library for AvidGPOrgs to use
    static const inst_lib_t & BaseInstLib(){
      static inst_lib_t inst_lib;
      if (inst_lib.GetSize() == 0) {
        inst_lib.AddInst("Inc", inst_lib_t::Inst_Inc, 1,
          "Increment value in reg Arg1");
        inst_lib.AddInst("Dec", inst_lib_t::Inst_Dec, 1,
          "Decrement value in reg Arg1");
        inst_lib.AddInst("Not", inst_lib_t::Inst_Not, 1,
          "Logically toggle value in reg Arg1");
        inst_lib.AddInst("SetReg", inst_lib_t::Inst_SetReg, 2,
          "Set reg Arg1 to numerical value Arg2");
        inst_lib.AddInst("Add", inst_lib_t::Inst_Add, 3,
          "regs: Arg3 = Arg1 + Arg2");
        inst_lib.AddInst("Sub", inst_lib_t::Inst_Sub, 3,
          "regs: Arg3 = Arg1 - Arg2");
        inst_lib.AddInst("Mult", inst_lib_t::Inst_Mult, 3,
          "regs: Arg3 = Arg1 * Arg2");
        inst_lib.AddInst("Div", inst_lib_t::Inst_Div, 3,
          "regs: Arg3 = Arg1 / Arg2");
        inst_lib.AddInst("Mod", inst_lib_t::Inst_Mod, 3,
          "regs: Arg3 = Arg1 % Arg2");
        inst_lib.AddInst("TestEqu", inst_lib_t::Inst_TestEqu, 3,
          "regs: Arg3 = (Arg1 == Arg2)");
        inst_lib.AddInst("TestNEqu", inst_lib_t::Inst_TestNEqu, 3,
          "regs: Arg3 = (Arg1 != Arg2)");
        inst_lib.AddInst("TestLess", inst_lib_t::Inst_TestLess, 3,
          "regs: Arg3 = (Arg1 < Arg2)");
        inst_lib.AddInst("If", inst_lib_t::Inst_If, 2,
          "If reg Arg1 != 0, scope -> Arg2; else skip scope", emp::ScopeType::BASIC, 1);
        inst_lib.AddInst("While", inst_lib_t::Inst_While, 2,
          "Until reg Arg1 != 0, repeat scope Arg2; else skip", emp::ScopeType::LOOP, 1);
        inst_lib.AddInst("Countdown", inst_lib_t::Inst_Countdown, 2,
          "Countdown reg Arg1 to zero; scope to Arg2", emp::ScopeType::LOOP, 1);
        inst_lib.AddInst("Break", inst_lib_t::Inst_Break, 1,
          "Break out of scope Arg1");
        inst_lib.AddInst("Scope", inst_lib_t::Inst_Scope, 1,
          "Enter scope Arg1", emp::ScopeType::BASIC, 0);
        inst_lib.AddInst("Define", inst_lib_t::Inst_Define, 2,
          "Build function Arg1 in scope Arg2", emp::ScopeType::FUNCTION, 1);
        inst_lib.AddInst("Call", inst_lib_t::Inst_Call, 1,
          "Call previously defined function Arg1");
        inst_lib.AddInst("Push", inst_lib_t::Inst_Push, 2,
          "Push reg Arg1 onto stack Arg2");
        inst_lib.AddInst("Pop", inst_lib_t::Inst_Pop, 2,
          "Pop stack Arg1 into reg Arg2");
        inst_lib.AddInst("Input", inst_lib_t::Inst_Input, 2,
          "Pull next value from input Arg1 into reg Arg2");
        //inst_lib.AddInst("Output", inst_lib_t::Inst_Output, 2,
        //	"Push reg Arg1 into output Arg2");
        inst_lib.AddInst("CopyVal", inst_lib_t::Inst_CopyVal, 2,
          "Copy reg Arg1 into reg Arg2");
        inst_lib.AddInst("ScopeReg", inst_lib_t::Inst_ScopeReg, 1,
          "Backup reg Arg1; restore at end of scope");
        inst_lib.AddInst("StartBirth", Inst_StartBirth, 0,
          "Begin replication");
        inst_lib.AddInst("Output", Inst_Output_Trig, 2,
          "Push reg Arg1 into output Arg2");
        inst_lib.AddInst("Nand", Inst_Nand, 3,
          "Perform the NAND logic operation");

        for (size_t i = 0; i < hardware_t::CPU_SIZE; i++) {
          inst_lib.AddArg(emp::to_string((int)i), i);                   // Args can be called by value
          inst_lib.AddArg(emp::to_string("Reg", 'A'+(char)i), i);  // ...or as a register.
        }
      }
      return inst_lib;
    }

  protected: 
    size_t birth_counter;
  public:
    AvidaGPOrg(OrganismManager<AvidaGPOrg> & _manager)
      : OrganismTemplate<AvidaGPOrg>(_manager),
      emp::AvidaCPU_Base<AvidaGPOrg>(BaseInstLib())
    {
      Initialize();
    }
    AvidaGPOrg(const AvidaGPOrg &) = default;
    AvidaGPOrg(AvidaGPOrg &&) = default;
    ~AvidaGPOrg() { ; }

    // Data shared by all AvidaGP organisms controlled by the same manager
    struct ManagerData : public Organism::ManagerData {
      double mut_prob = 0.01;            ///< Probability of each bit mutating on reproduction.
      std::string input_name = "inputs";    ///< Name of trait that should be used to access inputs
      std::string output_name = "outputs";  ///< Name of trait that should be used to access outputs
      size_t base_size = 10;             ///< Default number of instructions in a random organism
    };

    // TODO: Return something actually helpful (genome?)
    /// Use "to_string" to convert. 
    std::string ToString() const override { return "<AvidaGPOrg instance>"; }

    // TODO: actually use the per-bit mutation rate from the config 
    size_t Mutate(emp::Random & random) override {
      size_t num_muts = random.GetUInt(2);
        for (uint32_t m = 0; m < num_muts; m++) {
          const uint32_t pos = random.GetUInt(genome.sequence.size());
          RandomizeInst(pos, random);
        }
        return num_muts;
    }
    
    void Initialize(){
      // Initialize the flags we're using to pass messages from cpu to this class to 0
      // TODO: Move away from this nasty flag hack (see todo above class definition) 
      birth_counter = 0;
    }

    /// Reset org and randomize all instructions
    void Randomize(emp::Random & random) override {
      Reset();
      PushRandom(random, SharedData().base_size);
      Initialize();
    }

    /// Execute a single instruction (for now defined toward the bottom of file for acces to anager)
    bool ProcessStep() override;

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each bit mutating on reproduction.");
      GetManager().LinkVar(SharedData().base_size, "base_size",
                      "Number of instructions in randomly created organisms");
      GetManager().LinkVar(SharedData().input_name, "input_name",
                      "Trait name used when inputs are shared to other modules");
      GetManager().LinkVar(SharedData().output_name, "output_name",
                      "Trait name used when outputs are shared to other modules");
    }

    void IncrementBirthCounter() { 
      birth_counter++;
    }

  };

  // TODO: This can be deprecated with new updates!
  // Originally, we create a unique OrganismManger derivation that gives us addition functionality. 
  //    Specifically we needed orgs to fire triggers, but to do so the manager to expose a way to do
  //    so organisms cannot directly access the main MABE object  
  class AvidaGPOrgManager : public OrganismManager<AvidaGPOrg>{
    public:
    AvidaGPOrgManager(MABE & in_control, const std::string & in_name, 
          const std::string & in_desc="")
      : OrganismManager(in_control, in_name, in_desc){}
    
    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      // FIXME: These are hardcoded to be inputs/outputs right now, should be set by config
      //          Complication: OrganismManger's attribute "data" is private so we can't reach it
      //          OrganismTemplate has access because it's a friend of OrganismManager... 
      AddOwnedTrait<std::unordered_map<int, double>>("inputs", "Map of organism's inputs", 
          std::unordered_map<int, double>());
      //AddOwnedTrait<std::unordered_map<int, double>>("outputs", "Map of organism's outputs", 
      //    std::unordered_map<int, double>());
      AddOwnedTrait<double>("outputs", "filler", 0);
      control.GetSignalControl().AddSignal<void(Organism&)>("evaluate_org");
    }
    
    /// Fires the ManualEval trigger in the main MABE object
    void TriggerManualEval(Organism & org){
      control.GetSignalControl().Trigger<Organism&>("evaluate_org", org);
    }
    
  };


  // TODO: Move away from trait flags for cpu->AvidaGPOrg communication (see todo above class decl)
  // TODO: Create a better system for tasks. Should be easy once main MABE object allows triggers
  //          to be called from modules
  /// Process a single instruction and handle any results
  bool mabe::AvidaGPOrg::ProcessStep(){ 
    Process(1); // Process the next instruction
    // Was there an output?
    //if(emp::AvidaCPU_Base<AvidaGPOrg>::GetTrait(1) >= 0){
    //  emp::AvidaCPU_Base<AvidaGPOrg>::SetTrait(1,0);
    //  mabe::Organism::SetVar<std::unordered_map<int, double>>("inputs", inputs);
    //  //SetVar<std::unordered_map<int, double>>("outputs", outputs);
    //  SetVar<double>("outputs", GetOutput(0));
    //  static_cast<AvidaGPOrgManager*>(&GetManager())->TriggerManualEval(*this);
    //}
    // Check replication
    // Currently InstStartBirth must be called 30 times before org replicates(to give room to optimize)
    // InstStartBirth increments this trait (giving room to optimize)
    if(birth_counter >= 30){ 
      birth_counter -= 30;
      return true; // Tell scheduler to replicate org
    }
    return false; 
  }

  // TODO: This can be deprecated and replaced with the typical org registration macro 
  // See comments on AvidaGPOrgManager class for more info 
  struct AvidaGPModuleRegistrar {
    ModuleInfo new_info;
    AvidaGPModuleRegistrar() {
      const std::string& desc = "Genetic programming focused Avida digital organism";
      new_info.name = "AvidaGPOrg";
      new_info.desc = desc;
      new_info.init_fun = [desc](MABE & control, const std::string & name) -> ConfigType & {
        return control.AddModule<AvidaGPOrgManager>(name, desc);
      };
      GetModuleInfo().insert(new_info);
    }
  };
  mabe::AvidaGPModuleRegistrar MABE_AvidaGPOrgManager_Registrar;

    
  void AvidaGPOrg::Inst_Output_Trig(hardware_t & hw, const inst_t & inst){
      // Save the data in the target reg to the specified output position.
      int output_id = (int) hw.regs[ inst.args[1] ];  // Grab ID from register.
      hw.outputs[output_id] = hw.regs[inst.args[0]];  // Copy target reg to appropriate output.
      // Prepare and fire signal to evaluate output
      hw.SetVar<std::unordered_map<int, double>>("inputs", hw.inputs);
      //SetVar<std::unordered_map<int, double>>("outputs", outputs);
      hw.SetVar<double>("outputs", hw.GetOutput(0));
      static_cast<AvidaGPOrgManager*>(&hw.GetManager())->TriggerManualEval(hw);
    }
}

#endif
