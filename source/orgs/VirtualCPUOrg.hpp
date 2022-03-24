/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  VirtualCPUOrg.hpp
 *  @brief An organism consisting of a linear sequence of instructions.
 *  
 *  Instructions are added via other modules. VirtualCPUOrgs will load all instructions that have been registered (via the config file) and add them to the instruction library. 
 *
 *  For now, the virtual hardware of the VirtualCPUOrg is based on Avidians from Avida2, including support for additional nops, labels, and expanded nop notation for math instructions.
 *
 *  @note Status: ALPHA
 *
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

  /// \brief A wrapper of the emp::VirtualCPU class to be used in MABE
  ///
  /// A wrapper class for emp::VirtualCPU that makes it a MABE module. 
  /// Every population in Avida has an associated ActionMap, and it is through this ActionMap
  /// that VCPUOrgs load their instructions. This allows the end user to compose the 
  /// instruction set in the MABE configuration file.  
  class VirtualCPUOrg : 
    public OrganismTemplate<VirtualCPUOrg>, 
    public emp::VirtualCPU<VirtualCPUOrg> {

  public: 
    using base_t = emp::VirtualCPU<VirtualCPUOrg>;
    using this_t = VirtualCPUOrg;
    using typename base_t::genome_t;
    using typename base_t::inst_lib_t;
    using typename base_t::data_t;
    using inst_func_t = std::function<void(this_t&, const this_t::inst_t&)>;

  public:
    VirtualCPUOrg(OrganismManager<VirtualCPUOrg> & _manager)
      : OrganismTemplate<VirtualCPUOrg>(_manager), VirtualCPU(genome_t(GetInstLib()) ){ }
    VirtualCPUOrg(const VirtualCPUOrg &) = default;
    VirtualCPUOrg(VirtualCPUOrg &&) = default;
    ~VirtualCPUOrg() { ; }

    /// \brief A simple struct containing all variables shared among all VirtualCPUOrgs. 
    ///
    /// This includes all the configuration variables, as well as internal variables (e.g., 
    /// variables used in calculating mutations)
    struct ManagerData : public Organism::ManagerData {
      // Configuration variables
      double mut_prob = 0.01;     ///< Probability of each bit mutating on reproduction.
      size_t init_length = 100;   ///< Length of new organisms.
      bool init_random = true;    ///< Should we randomize ancestor?  (false = all zeros)
      size_t eval_time = 500;     ///< How long should the CPU be given on each evaluate?
      std::string input_name = "input";   /**< Name of trait that should be used to 
                                               load input values */
      std::string output_name = "output"; /**< Name of trait that should be used to store 
                                               output values */
      std::string merit_name = "merit";   /**< Name of trait that stores the merit of an org 
                                               as it was passed from its parent */ 
      std::string genome_name = "genome"; ///< Name of trait that stores an org's genome 
      std::string child_merit_name = "child_merit"; /**< Name of the trait that stores an 
                                                         org's merit during its lifetime. This
                                                         is then passed to its offspring */
      std::string generation_name = "generation";  /**<  Name of the trait that store's the 
                                                         org's generation */
      double initial_merit = 0; ///< Merit that the ancestor starts with
      bool verbose = false;     ///< Flag that indicates whether to print additional info
      std::string initial_genome_filename = "ancestor.org"; /**< If init_random is false, this
                                                                 indicates a file that 
                                                                 contains the ancestor's 
                                                                 genome */
      bool expanded_nop_args = false; /**< Flag that indicates whether to use the "expanded
                                           nop" syntax. If true, instructions like and can 
                                           take up to three nops to specify all three of the 
                                           following: a + b = c */
      // Internal use
      emp::Binomial mut_dist;         ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;       ///< A pre-allocated vector for mutation sites. 
    };

    /// Mutate (in place) the current organism. Currently only supports point mutations.
    size_t Mutate(emp::Random & random) override {
      const size_t num_muts = SharedData().mut_dist.PickRandom(random);

      if(num_muts == 0) return 0;
      if(num_muts == 1){
        const size_t pos = random.GetUInt(GetGenomeSize());
        RandomizeInst(pos, random);
        return 1;
      }
      // Only remaining option is num_muts > 1.
      emp::BitVector mut_sites(genome.size());
      for (size_t i = 0; i < num_muts; i++) {
        const size_t pos = random.GetUInt(GetGenomeSize());
        if(mut_sites[pos]){ --i; continue; } // Duplicate position; try again.
        mut_sites[pos] = true;
        size_t old_inst_idx = genome[pos].idx;
        RandomizeInst(pos, random);
        while(genome[pos].idx == old_inst_idx) RandomizeInst(pos, random);
      }
      return num_muts;
    }

    /// Randomize (in place) the organism's genome. Does not add new instructions.
    void Randomize(emp::Random & random) override {
      for (size_t pos = 0; pos < GetGenomeSize(); pos++) {
        RandomizeInst(pos, random);
      }
      Organism::SetTrait<std::string>(SharedData().genome_name, GetGenomeString());
    }

    /// Pad the organism's genome out to the specified length with random instructions.
    void FillRandom(size_t length, emp::Random & random){
      for (size_t pos = GetGenomeSize(); pos < length; pos++) {
        PushRandomInst(random);
      }
    }

    /// Create an ancestral organism and load in values from configuration file
    void Initialize(emp::Random & random) override {
      emp_assert(GetGenomeSize() == 0, "Cannot initialize VirtualCPUOrg twice");
      // Create the ancestor, either randomly or from a genome file
      if(SharedData().init_random) FillRandom(SharedData().init_length, random);
      else Load(SharedData().initial_genome_filename);
      // Update values based on configuration variables
      expanded_nop_args = SharedData().expanded_nop_args;
      Organism::SetTrait<std::string>(SharedData().genome_name, GetGenomeString());
      Organism::SetTrait<double>(SharedData().merit_name, SharedData().initial_merit); 
      Organism::SetTrait<double>(SharedData().child_merit_name, SharedData().initial_merit); 
      Organism::SetTrait<size_t>(SharedData().generation_name, 0); 
      base_t::Initialize(); // MABE's proto organisms means we need to re-initialize the org
      CurateNops();
    }
    
    /// Create an offspring organism using the configuration file's mutation rate.
    emp::Ptr<Organism> MakeOffspringOrganism(emp::Random & random) const {
      // Create and mutate
      auto offspring = OrgType::Clone().DynamicCast<VirtualCPUOrg>();
      offspring->Mutate(random);
      // Initialize all necessary traits and ready hardware
      offspring->SetTrait<double>(SharedData().merit_name, GetTrait<double>(SharedData().child_merit_name)); 
      offspring->SetTrait<double>(SharedData().child_merit_name, SharedData().initial_merit); 
      offspring->SetTrait<size_t>(SharedData().generation_name, 
          GetTrait<size_t>(SharedData().generation_name) + 1); 
      offspring->CurateNops();
      offspring->Organism::SetTrait<std::string>(
          SharedData().genome_name, offspring->GetGenomeString());
      offspring.DynamicCast<VirtualCPUOrg>()->ResetHardware();
      return offspring;
    }
    
    /// Create an identical organism with no mutations and with the same merit
    virtual emp::Ptr<Organism> CloneOrganism() const {
      auto offspring = OrgType::Clone().DynamicCast<VirtualCPUOrg>();
      offspring->genome = genome;
      offspring->SetTrait<double>(SharedData().merit_name, GetTrait<double>(SharedData().merit_name)); 
      offspring->SetTrait<double>(SharedData().child_merit_name, SharedData().initial_merit); 
      offspring->genome_working = offspring->genome;
      offspring->ResetHardware();
      offspring->Organism::SetTrait<std::string>(SharedData().genome_name, offspring->GetGenomeString());
      offspring->expanded_nop_args = SharedData().expanded_nop_args;
      return offspring;
    }

    /// Load inputs and run the organism for a number of steps specified in the configuration
    /// file. Any generated outputs will be stored in the organism's output trait.
    void GenerateOutput() override {
      ResetHardware();

      // Setup the input.
      SetInputs(Organism::GetTrait<emp::vector<data_t>>(SharedData().input_name));

      // Run the code.
      Process(SharedData().eval_time, SharedData().verbose);
    }

    /// Return a reference to the instruction library of the organism
    static inst_lib_t& GetInstLib(){
      static inst_lib_t inst_lib;
      return inst_lib;
    }

    /// Set up configuration options for this organism type
    void SetupConfig() override {
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each instruction mutating on reproduction.");
      GetManager().LinkFuns<size_t>([this](){ return GetGenomeSize(); },
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
      GetManager().LinkVar(SharedData().generation_name, "generation_name",
                      "Name of variable corresponding to the organism's generation. "
                      "When an organism replicates, the child's gen. is the parent's gen +1");
      GetManager().LinkVar(SharedData().initial_merit, "inititial_merit",
                      "Initial value for merit (task performance)");
      GetManager().LinkVar(SharedData().verbose, "verbose",
                      "If true, print execution info of organisms");
      GetManager().LinkVar(SharedData().initial_genome_filename, "initial_genome_filename",
                      "File that contains the gennome used to initialize organisms.");
      GetManager().LinkVar(SharedData().expanded_nop_args, "expanded_nop_args",
                      "If true, some instructions (e.g., math) will use multiple nops to fully "
                      "define the registers used");
    }

    /// Set up this organism type with the traits it need to track and initialize 
    /// shared variables.
    void SetupModule() override {
      SetupMutationDistribution();
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
      GetManager().AddOwnedTrait<size_t>(SharedData().generation_name, "Organism's generation", 0);
      SetupInstLib();
    }

    /// Load external instructions that were added via the configuration file
    void SetupInstLib(){
      inst_lib_t& inst_lib = GetInstLib();
      // All instructions are stored in the populations ActionMap
      ActionMap& action_map = GetManager().GetControl().GetActionMap(0);
      std::unordered_map<std::string, mabe::Action>& typed_action_map =
        action_map.GetFuncs<void, VirtualCPUOrg&, const inst_t&>();
      // Print the number of instructions found and each of their names
      std::cout << "Found " << typed_action_map.size() << " external functions!";
      for(auto it = typed_action_map.begin(); it != typed_action_map.end(); it++){
        std::cout << " " << it->first;
      }
      std::cout << std::endl;
      // Iterate over each instruction that was found, and add it to the instruction library
      for(auto it = typed_action_map.begin(); it != typed_action_map.end(); it++){
        mabe::Action& action = it->second; 
        emp_assert(action.data.HasName("inst_id"), 
            "Instructions must provide an inst_id:", action.name);
        emp_assert(action.data.IsType<int>("inst_id"),
            "Instruction's inst_id must be an int!");
        // Calculate the character associated with this instruction 
        unsigned char c = 'a' + action.data.Get<int>("inst_id");
        if(action.data.Get<int>("inst_id") > 25){
          c = 'A' + action.data.Get<int>("inst_id") - 26;
        }
        std::cout << "Found " << action.function_vec.size() << 
            " external functions with name: " << action.name << "!" <<
            " (" << c << ")" << std::endl;
        // Grab description
        const std::string desc = 
          (action.data.HasName("description") ? 
            action.data.Get<std::string>("description") : "No description provided");
        const size_t num_args = 
          (action.data.HasName("num_args") ?  action.data.Get<size_t>("num_args") : 0);
        inst_lib.AddInst(
            action.name,                       // Instruction name
            [&action](VirtualCPUOrg& org, const inst_t& inst){
              for(size_t func_idx = 0; func_idx < action.function_vec.size(); ++func_idx){
                action.function_vec[func_idx].Call<void, VirtualCPUOrg&, const inst_t&>(org, inst);
              }
            },                                 // Function that will be executed
            num_args,                          // Number of arguments
            desc,                              // Description 
            emp::ScopeType::NONE,              // No scope type, but must provide
            (size_t) -1,                       // Scope arg, must provide 
            std::unordered_set<std::string>(), // Instruction properties
            action.data.Get<int>("inst_id"));  // Instruction ID
      }
    }

    /// Process a single instruction
    bool ProcessStep() override { 
      Process(1, SharedData().verbose);
      return true; 
    }

    /// Initialize the mutational distribution variables to match the genome size (either 
    /// current size or projected sizes)
    void SetupMutationDistribution(){
      if(GetGenomeSize() != 0){ // If we have a genome size, use it!
        SharedData().mut_dist.Setup(SharedData().mut_prob, GetGenomeSize());
        SharedData().mut_sites.Resize(GetGenomeSize());
      }
      else{ // Otherwise, use the genome size set in the configuration file
        SharedData().mut_dist.Setup(SharedData().mut_prob, SharedData().init_length);
        SharedData().mut_sites.Resize(SharedData().init_length);
      }
    }

  };

  MABE_REGISTER_ORG_TYPE(VirtualCPUOrg, "Organism consisting of Avida instructions.");
}

#endif
