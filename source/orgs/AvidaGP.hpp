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
#include "./AvidaGP_InstLib.hpp"

#include "emp/math/Distribution.hpp"
#include "emp/math/random_utils.hpp"

  namespace mabe {

    // Note: typedefs currently in AvidaGP_InstLib for reuse
    
    //TODO: Shift this class from "has-a" AvidaGP to "is-a" AvidaGP 
    //        This will allow for instructions to change code inside this class and bypass the need to 
    //        store flags in the cpu 

    class AvidaGPOrg : public OrganismTemplate<AvidaGPOrg> {
    protected:
      hardware_t cpu; ///< Virtual cpu (Empirical's AvidaGP 

    public:
      AvidaGPOrg(OrganismManager<AvidaGPOrg> & _manager)
        : OrganismTemplate<AvidaGPOrg>(_manager),
          cpu(BaseInstLib())
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
            const uint32_t pos = random.GetUInt(cpu.genome.sequence.size());
            cpu.RandomizeInst(pos, random);
          }
          return num_muts;
      }
      
      void Initialize(){
        // Initialize the flags we're using to pass messages from cpu to this class to 0
        // TODO: Move away from this nasty flag hack (see todo above class definition) 
        cpu.SetTrait(0, 0);
        cpu.SetTrait(1, 0);
      }

      /// Reset org and randomize all instructions
      void Randomize(emp::Random & random) override {
        cpu.Reset();
        cpu.PushRandom(random, SharedData().base_size);
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
      cpu.Process(1); // Process the next instruction
      // Was there an output?
      if(cpu.GetTrait(1) >= 0){
        cpu.SetTrait(1,0);
        SetVar<std::unordered_map<int, double>>("inputs", cpu.inputs);
        //SetVar<std::unordered_map<int, double>>("outputs", cpu.outputs);
        SetVar<double>("outputs", cpu.GetOutput(0));
        static_cast<AvidaGPOrgManager*>(&GetManager())->TriggerManualEval(*this);
      }
      // Check replication
      // Currently InstStartBirth must be called 30 times before org replicates(to give room to optimize)
      if(cpu.GetTrait(0) >= 30){ // InstStartBirth increments this trait (giving room to optimize)
        cpu.SetTrait(0,0);
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
}

#endif
