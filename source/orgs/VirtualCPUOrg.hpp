/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021.
 *
 *  @file  VirtualCPUOrg.hpp
 *  @brief An organism consisting of linear code.
 *  @note Status: ALPHA
 */

#ifndef MABE_VIRTUAL_CPU_ORGANISM_H
#define MABE_VIRTUAL_CPU_ORGANISM_H

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"

#include "emp/datastructs/vector_utils.hpp"
#include "emp/hardware/VirtualCPU.hpp"
#include "emp/math/Distribution.hpp"
#include "emp/math/random_utils.hpp"

namespace mabe {

  class VirtualCPUOrg : 
    public OrganismTemplate<VirtualCPUOrg>, 
    public emp::VirtualCPU<VirtualCPUOrg> {

  public: 
    using base_t = emp::VirtualCPU<VirtualCPUOrg>;
    using typename base_t::genome_t;
    using typename base_t::inst_lib_t;
    using typename base_t::data_t;

  public:
    VirtualCPUOrg(OrganismManager<VirtualCPUOrg> & _manager)
      : OrganismTemplate<VirtualCPUOrg>(_manager), VirtualCPU(genome_t(GetInstLib()) ){ }
    VirtualCPUOrg(const VirtualCPUOrg &) = default;
    VirtualCPUOrg(VirtualCPUOrg &&) = default;
    ~VirtualCPUOrg() { ; }

    struct ManagerData : public Organism::ManagerData {
      // Configuration variables
      double mut_prob = 0.01;              ///< Probability of each bit mutating on reproduction.
      size_t init_length = 100;            ///< Length of new organisms.
      bool init_random = true;             ///< Should we randomize ancestor?  (false = all zeros)
      size_t eval_time = 500;              ///< How long should the CPU be given on each evaluate?
      std::string input_name = "input";    ///< Name of trait that should be used load input values
      std::string output_name = "output";  ///< Name of trait that should be used store output values
      std::string merit_name = "merit";    ///< Name of trait that stores an organism's fitness 
      std::string genome_name = "genome";    ///< Name of trait that stores an organism's fitness 
      std::string child_merit_name = "child_merit"; 
      double initial_merit = 0;

      // Internal use
      emp::Binomial mut_dist;            ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;            ///< A pre-allocated vector for mutation sites. 
    };

    size_t Mutate(emp::Random & random) override {
      emp::Binomial mut_dist(SharedData().mut_prob, genome.size());
      //const size_t num_muts = SharedData().mut_dist.PickRandom(random);
      const size_t num_muts = mut_dist.PickRandom(random);

      if (num_muts == 0) return 0;
      if (num_muts == 1) {
        const size_t pos = random.GetUInt(GetSize());
        RandomizeInst(pos, random);
        return 1;
      }

      // Only remaining option is num_muts > 1.
      emp::BitVector mut_sites(genome.size());
      for (size_t i = 0; i < num_muts; i++) {
        const size_t pos = random.GetUInt(GetSize());
        if (mut_sites[pos]) { --i; continue; }  // Duplicate position; try again.
        RandomizeInst(pos, random);
      }
      return num_muts;
    }

    void Randomize(emp::Random & random) override {
      for (size_t pos = 0; pos < GetSize(); pos++) {
        RandomizeInst(pos, random);
      }
      Organism::SetTrait<std::string>(SharedData().genome_name, GetString());
    }

    void Initialize(emp::Random & random) override {
      std::cout << "Original genome:" << std::endl;
      std::cout << GetString() << std::endl;
      constexpr bool start_with_not = false;
      if (SharedData().init_random) Randomize(random);
      else{
        std::cout << "Filling genome!" << std::endl;
        PushInst("HAlloc");  // 0  
        PushInst("HSearch"); // 1
        PushInst("NopC");    // 2
        PushInst("NopA");    // 3
        PushInst("MovHead"); // 4
        if constexpr(!start_with_not){
          for(size_t i = 0; i < (50 - 14); i++) PushInst("NopC");   
        }
        else{
          for(size_t i = 0; i < (50 - 24); i++) PushInst("NopC");   
          PushInst("IO");
          PushInst("NopB");
          PushInst("Push");
          PushInst("NopB");
          PushInst("Pop");
          PushInst("NopC");
          PushInst("Nand");
          PushInst("NopB");
          PushInst("IO");
          PushInst("NopB");
        }
        PushInst("HSearch"); // 41
        PushInst("HCopy");   // 42
        PushInst("IfLabel"); // 43
        PushInst("NopC");    // 44
        PushInst("NopA");    // 45
        PushInst("HDivide"); // 46
        PushInst("MovHead"); // 47
        PushInst("NopA");    // 48
        PushInst("NopB");    // 49
      }
      Organism::SetTrait<std::string>(SharedData().genome_name, GetString());
      Organism::SetTrait<double>(SharedData().merit_name, SharedData().initial_merit); 
      Organism::SetTrait<double>(SharedData().child_merit_name, SharedData().initial_merit); 
      CurateNops();
      std::cout << "Modified genome:" << std::endl;
      std::cout << GetString() << std::endl;
    }
    
    emp::Ptr<Organism> MakeOffspringOrganism(emp::Random & random) const {
      emp::Ptr<Organism> offspring = CloneOrganism();
      offspring->Mutate(random);
      offspring->SetTrait<double>(SharedData().merit_name, GetTrait<double>(SharedData().child_merit_name)); 
      offspring->SetTrait<double>(SharedData().child_merit_name, SharedData().initial_merit); 
      offspring.DynamicCast<VirtualCPUOrg>()->CurateNops();
      offspring.DynamicCast<VirtualCPUOrg>()->Organism::SetTrait<std::string>(
          SharedData().genome_name, offspring.DynamicCast<VirtualCPUOrg>()->GetString());
      return offspring;
    }
    
    virtual emp::Ptr<Organism> CloneOrganism() const {
      auto offspring = OrgType::Clone().DynamicCast<VirtualCPUOrg>();
      //std::cout << 
      //  GetTrait<OrgPosition>("org_pos").Pos() << 
      //  ConvertGenome(genome_working) << 
      //  " -> " << 
      //  ConvertGenome(GetTrait<genome_t>("offspring_genome")) << 
      //  std::endl; 
      offspring->genome = GetTrait<genome_t>("offspring_genome");
      offspring->SetTrait<double>(SharedData().merit_name, GetTrait<double>(SharedData().child_merit_name)); 
      offspring->SetTrait<double>(SharedData().child_merit_name, SharedData().initial_merit); 
      offspring->genome_working = offspring->genome;
      offspring->ResetHardware();
      offspring->Organism::SetTrait<std::string>(SharedData().genome_name, offspring->GetString());
      return offspring;
    }

    /// Put the output values in the correct output position.
    void GenerateOutput() override {
      ResetHardware();

      // Setup the input.
      SetInputs(Organism::GetTrait<emp::vector<data_t>>(SharedData().input_name));

      // Run the code.
      Process(SharedData().eval_time);

      // Store the results.
      Organism::SetTrait<emp::vector<data_t>>(SharedData().output_name, emp::ToVector( GetOutputs() ));
    }

    static inst_lib_t& GetInstLib(){
      static inst_lib_t inst_lib;
      return inst_lib;
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each instruction mutating on reproduction.");
      GetManager().LinkFuns<size_t>([this](){ return GetSize(); },
                       [this](const size_t & N){ Reset(); /*PushDefaultInst(N);*/ },
                       "N", "Initial number of instructions in genome");
      GetManager().LinkVar(SharedData().init_random, "init_random",
                      "Should we randomize ancestor?  (0 = \"blank\" default)");
      GetManager().LinkVar(SharedData().eval_time, "eval_time",
                      "How many CPU cycles should we give organisms to run?");
      GetManager().LinkVar(SharedData().input_name, "input_name",
                      "Name of variable to load inputs from.");
      GetManager().LinkVar(SharedData().output_name, "output_name",
                      "Name of variable to output results.");
      GetManager().LinkVar(SharedData().genome_name, "genome_name",
                      "Where to store the genome?.");
      GetManager().LinkVar(SharedData().merit_name, "merit_name",
                      "Name of variable corresponding to the organism's task performance.");
      GetManager().LinkVar(SharedData().child_merit_name, "child_merit_name",
                      "Name of variable corresponding to the organism's task performance that"
                      " will be used to calculate CPU cylces given to offspring.");
      GetManager().LinkVar(SharedData().initial_merit, "inititial_merit",
                      "Initial value for merit (task performance)");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      // Setup the mutation distribution.
      SharedData().mut_dist.Setup(SharedData().mut_prob, 20);

      // Setup the default vector to indicate mutation positions.
      SharedData().mut_sites.Resize(GetSize());

      // Setup the input and output traits.
      GetManager().AddRequiredTrait<emp::vector<data_t>>(SharedData().input_name);
      GetManager().AddSharedTrait(SharedData().output_name,
                                  "Value map output from organism.",
                                  emp::vector<data_t>());
      GetManager().AddSharedTrait<double>(SharedData().merit_name,
                                  "Value representing fitness of organism", SharedData().initial_merit);
      GetManager().AddSharedTrait<double>(SharedData().child_merit_name,
                                  "Fitness passed on to children", SharedData().initial_merit);
      GetManager().AddOwnedTrait<std::string>(SharedData().genome_name, "Organism's genome", "[None]");
      GetManager().AddSharedTrait<genome_t>("offspring_genome", "Latest genome copied", { } );
      GetManager().AddSharedTrait<genome_t>("passed_genome", "Genome as passed from parent", { } );
      SetupInstLib();
    }

    void SetupInstLib(){
      /*
      inst_lib_t& inst_lib = GetInstLib();
      inst_lib.AddInst("Inc", inst_lib_t::Inst_Inc, 1, "Increment value in reg Arg1");
      inst_lib.AddInst("Dec", inst_lib_t::Inst_Dec, 1, "Decrement value in reg Arg1");
      inst_lib.AddInst("Not", inst_lib_t::Inst_Not, 1, "Logically toggle value in reg Arg1");
      inst_lib.AddInst("SetReg", inst_lib_t::Inst_SetReg, 2, "Set reg Arg1 to numerical value Arg2");
      inst_lib.AddInst("Add", inst_lib_t::Inst_Add, 3, "regs: Arg3 = Arg1 + Arg2");
      inst_lib.AddInst("Sub", inst_lib_t::Inst_Sub, 3, "regs: Arg3 = Arg1 - Arg2");
      inst_lib.AddInst("Mult", inst_lib_t::Inst_Mult, 3, "regs: Arg3 = Arg1 * Arg2");
      inst_lib.AddInst("Div", inst_lib_t::Inst_Div, 3, "regs: Arg3 = Arg1 / Arg2");
      inst_lib.AddInst("Mod", inst_lib_t::Inst_Mod, 3, "regs: Arg3 = Arg1 % Arg2");
      inst_lib.AddInst("TestEqu", inst_lib_t::Inst_TestEqu, 3, "regs: Arg3 = (Arg1 == Arg2)");
      inst_lib.AddInst("TestNEqu", inst_lib_t::Inst_TestNEqu, 3, "regs: Arg3 = (Arg1 != Arg2)");
      inst_lib.AddInst("TestLess", inst_lib_t::Inst_TestLess, 3, "regs: Arg3 = (Arg1 < Arg2)");
      inst_lib.AddInst("If", inst_lib_t::Inst_If, 2, "If reg Arg1 != 0, scope -> Arg2; else skip scope", emp::ScopeType::BASIC, 1);
      inst_lib.AddInst("While", inst_lib_t::Inst_While, 2, "Until reg Arg1 != 0, repeat scope Arg2; else skip", emp::ScopeType::LOOP, 1);
      inst_lib.AddInst("Countdown", inst_lib_t::Inst_Countdown, 2, "Countdown reg Arg1 to zero; scope to Arg2", emp::ScopeType::LOOP, 1);
      inst_lib.AddInst("Break", inst_lib_t::Inst_Break, 1, "Break out of scope Arg1");
      inst_lib.AddInst("Scope", inst_lib_t::Inst_Scope, 1, "Enter scope Arg1", emp::ScopeType::BASIC, 0);
      inst_lib.AddInst("Define", inst_lib_t::Inst_Define, 2, "Build function Arg1 in scope Arg2", emp::ScopeType::FUNCTION, 1);
      inst_lib.AddInst("Call", inst_lib_t::Inst_Call, 1, "Call previously defined function Arg1");
      inst_lib.AddInst("Push", inst_lib_t::Inst_Push, 2, "Push reg Arg1 onto stack Arg2");
      inst_lib.AddInst("Pop", inst_lib_t::Inst_Pop, 2, "Pop stack Arg1 into reg Arg2");
      inst_lib.AddInst("Input", inst_lib_t::Inst_Input, 2, "Pull next value from input Arg1 into reg Arg2");
      inst_lib.AddInst("Output", inst_lib_t::Inst_Output, 2, "Push reg Arg1 into output Arg2");
      inst_lib.AddInst("CopyVal", inst_lib_t::Inst_CopyVal, 2, "Copy reg Arg1 into reg Arg2");
      inst_lib.AddInst("ScopeReg", inst_lib_t::Inst_ScopeReg, 1, "Backup reg Arg1; restore at end of scope");

      for (size_t i = 0; i < VirtualCPUOrg::CPU_SIZE; i++) {
        GetInstLib().AddArg(emp::to_string((int)i), i);                   // Args can be called by value
        GetInstLib().AddArg(emp::to_string("Reg", 'A'+(char)i), i);  // ...or as a register.
      }
      */
      LoadExternalInstructions();
    }

    void LoadExternalInstructions(){
      inst_lib_t& inst_lib = GetInstLib();
      ActionMap& action_map = GetManager().GetControl().GetActionMap(0);
      std::unordered_map<std::string, mabe::Action>& typed_action_map =
        action_map.GetFuncs<void, VirtualCPUOrg&, const inst_t&>();
      std::cout << "Found " << typed_action_map.size() << " external functions!" << std::endl;

      for(auto it = typed_action_map.begin(); it != typed_action_map.end(); it++){
        std::cout << " " << it->first;
      }
      std::cout << std::endl;
      for(auto it = typed_action_map.begin(); it != typed_action_map.end(); it++){
        mabe::Action& action = it->second; 
        std::cout << "Found " << action.function_vec.size() << 
            " external functions with name: " << action.name << "!" << std::endl;
        inst_lib.AddInst(
            action.name,
            //func,
            [&action](VirtualCPUOrg& org, const inst_t& inst){
              //std::cout << action.name << std::endl;
              for(size_t func_idx = 0; func_idx < action.function_vec.size(); ++func_idx){
                action.function_vec[func_idx].Call<void, VirtualCPUOrg&, const inst_t&>(org, inst);
              }
            },
            action.num_args, 
            "test", 
            action.data.Get<int>("inst_id"));
        std::cout << "Added instruction: " << action.name << std::endl;
      }
    }

    bool ProcessStep() override { 
      //std::cout << GetString() << std::endl;
      //if(GetTrait<OrgPosition>("org_pos").Pos() == 1){
      //  std::cout 
      //      << GetTrait<OrgPosition>("org_pos").Pos() 
      //      << " -> " 
      //      << inst_ptr 
      //      << " (" << GetInstLib().GetName(genome_working[inst_ptr].idx) << ")"
      //      << std::endl; 
      //}
      Process(1);
      return true; 
    }

      static std::string ConvertGenome(const genome_t& genome){
        std::stringstream sstr;
        sstr << "[" << genome.size() << "]";
        for(size_t idx = 0; idx < genome.size(); idx++){
          unsigned char c = 'a' + genome[idx].id;
          sstr << c;
        }
        return sstr.str();
      }
  };

  MABE_REGISTER_ORG_TYPE(VirtualCPUOrg, "Organism consisting of Avida instructions.");
}

#endif