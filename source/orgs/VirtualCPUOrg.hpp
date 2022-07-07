/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  VirtualCPUOrg.hpp
 *  @brief An organism consisting of a linear sequence of instructions.
 *  
 *  Instructions are added via other modules. 
 *    VirtualCPUOrgs will load all instructions that have been registered 
 *    (via the config file) and add them to the instruction library. 
 *
 *  For now, the virtual hardware of the VirtualCPUOrg is based on Avidians from Avida2, 
 *    including support for additional nops, labels, and expanded nop notation for 
 *    math instructions.
 *
 *  @note Status: ALPHA
 *
 *  TODO: 
 *    - Decide what to do with N config option
 *      - Is it okay to have it readonly?
 *
 */

#ifndef MABE_VIRTUAL_CPU_ORGANISM_H
#define MABE_VIRTUAL_CPU_ORGANISM_H

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"

#include "emp/datastructs/vector_utils.hpp"
#include "emp/hardware/VirtualCPU.hpp"
#include "emp/math/CombinedBinomialDistribution.hpp"
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

  protected: 
    size_t insts_speculatively_executed = 0;
    emp::BitVector non_speculative_inst_vec;
    

    /// Perform a single point mutation at the given position
    void Mutate_Point(size_t pos, emp::Random& random){
      size_t old_inst_idx = genome[pos].idx;
      RandomizeInst(pos, random);
      while(genome[pos].idx == old_inst_idx) RandomizeInst(pos, random);
    }

    /// Perform a single insertion mutation at the given position
    void Mutate_Insertion(size_t pos, emp::Random& random){
      InsertRandomInst(pos, random);
    }

    /// Perform a single deletion mutation at the given position
    void Mutate_Deletion(size_t pos, emp::Random& /*random*/){
      RemoveInst(pos);
    }

    /// Apply mutations according to the passed parameters, and then call the given function 
    /// for each mutation
    size_t Mutate_Generic(
        std::function<void(size_t, emp::Random&)> mut_func,
        emp::CombinedBinomialDistribution& dist, 
        emp::Random& random, 
        bool ensure_unique_pos = true){
      const size_t num_muts = dist.PickRandom(GetGenomeSize(), random);

      if(num_muts == 0){
        return 0;
      }
      if(num_muts == 1){
        const size_t pos = random.GetUInt(GetGenomeSize());
        mut_func(pos, random);
        return 1;
      }
      // Only remaining option is num_muts > 1.
      if(ensure_unique_pos){ // Ensure no two mutations hit the same site
        emp::BitVector mut_sites(genome.size());
        for (size_t i = 0; i < num_muts; i++) {
          const size_t pos = random.GetUInt(GetGenomeSize());
          if(mut_sites[pos]){ --i; continue; } // Duplicate position; try again.
          mut_sites[pos] = true;
          mut_func(pos, random);
          if(mut_sites.size() != GetGenomeSize()){
            mut_sites.resize(GetGenomeSize());
          }
        }
      }
      else{ // Mutate without concern of mutations hitting the same site (e.g., deletion)
        for (size_t i = 0; i < num_muts; i++) {
          const size_t pos = random.GetUInt(GetGenomeSize());
          mut_func(pos, random);
        }
      }
      return num_muts;
    }

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
      double point_mut_prob = 0.01;      ///< Per-site point mutation rate.
      double insertion_mut_prob = 0.01;  ///< Per-site insertion mutation rate.
      double deletion_mut_prob = 0.01;   ///< Per-site deletion mutation rate.
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
      std::string offspring_genome_name = "offspring_genome";  /**< Name of trait that stores 
                                                                    the genome for the 
                                                                    upcoming offspring */
      std::string position_name = "org_pos"; ///< Name of trait that stores org's position 
      std::string genome_length_name = "genome_length"; /**< Name of trait that stores length 
                                                             of org's genome 
                                                   */
      std::string child_merit_name = "child_merit"; /**< Name of the trait that stores an 
                                                         org's merit during its lifetime. This
                                                         is then passed to its offspring */
      std::string generation_name = "generation";  /**<  Name of the trait that store's the 
                                                         org's generation */
      double initial_merit = 0; ///< Merit that the ancestor starts with
      bool copy_influences_merit = true;  /**<  Does the number of instructions copied 
                                                  influence merit passed to offspring? */
      bool verbose = false;     ///< Flag that indicates whether to print additional info
      std::string initial_genome_filename = "ancestor.org"; /**< If init_random is false, this
                                                                 indicates a file that 
                                                                 contains the ancestor's 
                                                                 genome */
      std::string inst_set_input_filename = ""; /// File to load for the instruction set
      std::string inst_set_output_filename = ""; /**< If not empty, writes instruction set
                                                      (in order) to the specified file **/
      bool expanded_nop_args = false; /**< Flag that indicates whether to use the "expanded
                                           nop" syntax. If true, instructions like and can 
                                           take up to three nops to specify all three of the 
                                           following: a + b = c */
      bool use_speculative_execution = false; /**< Flag indicating whether we speculatively 
                                                   execute instruction.*/
      int max_speculative_insts = -1;         /**< Maximum number of insts. to speculatively 
                                                  execute. -1 for genome length. */
      // Internal use
      emp::CombinedBinomialDistribution point_mut_dist; ///< Distribution of number of point mutations to occur.
      emp::CombinedBinomialDistribution insertion_mut_dist; ///< Distribution of number of insertion mutations to occur.
      emp::CombinedBinomialDistribution deletion_mut_dist; ///< Distribution of number of deletion mutations to occur.
      emp::BitVector mut_sites; ///< A pre-allocated vector for mutation sites. 
    };

    /// Mutate (in place) the current organism.
    size_t Mutate(emp::Random & random) override {
      size_t mut_count = 0;
      mut_count += Mutate_Generic(
        [this](size_t pos, emp::Random& random){
          Mutate_Point(pos, random);
        },
        SharedData().point_mut_dist, random, true
      );
      mut_count += Mutate_Generic(
        [this](size_t pos, emp::Random& random){
          Mutate_Insertion(pos, random);
        },
        SharedData().insertion_mut_dist, random, false 
      );
      mut_count += Mutate_Generic(
        [this](size_t pos, emp::Random& random){
          Mutate_Deletion(pos, random);
        },
        SharedData().deletion_mut_dist, random, false
      );
      // Update hardware and traits accordingly
      ResetWorkingGenome();
      Organism::SetTrait<std::string>(SharedData().genome_name, GetGenomeString());
      Organism::SetTrait<size_t>(SharedData().genome_length_name, GetGenomeSize());
      return mut_count;
    }

    /// Randomize (in place) the organism's genome. Does not add new instructions.
    void Randomize(emp::Random & random) override {
      for (size_t pos = 0; pos < GetGenomeSize(); pos++) {
        RandomizeInst(pos, random);
      }
      ResetWorkingGenome();
      Organism::SetTrait<std::string>(SharedData().genome_name, GetGenomeString());
      Organism::SetTrait<size_t>(SharedData().genome_length_name, GetGenomeSize());
    }

    /// Pad the organism's genome out to the specified length with random instructions.
    void FillRandom(size_t length, emp::Random & random){
      for (size_t pos = GetGenomeSize(); pos < length; pos++) {
        PushRandomInst(random);
      }
      ResetWorkingGenome();
    }

    /// Reset organism's hardware to the top of the original genome
    void ResetHardware(){
      ResetWorkingGenome();
      expanded_nop_args = SharedData().expanded_nop_args;
      base_t::Initialize();
      insts_speculatively_executed = 0;
      CurateNops();
    }
      
    /// Reset organism's traits to match what it was born with
    void ResetTraits(){
      const double merit = Organism::GetTrait<double>(SharedData().merit_name);
      const size_t gen = Organism::GetTrait<size_t>(SharedData().generation_name);
      const OrgPosition pos = Organism::GetTrait<OrgPosition>(SharedData().position_name);
      GetManager().GetControl().ResetTraits(*this);
      Organism::SetTrait<double>(SharedData().merit_name, merit);
      Organism::SetTrait<size_t>(SharedData().generation_name, gen);
      Organism::SetTrait<OrgPosition>(SharedData().position_name, pos);
      Organism::SetTrait<std::string>(SharedData().genome_name, GetGenomeString());
      Organism::SetTrait<size_t>(SharedData().genome_length_name, GetGenomeSize());
      Organism::SetTrait<double>(SharedData().child_merit_name, 
          SharedData().initial_merit); 
    }

    /// Create an ancestral organism and load in values from configuration file
    void Initialize(emp::Random & random) override {
      emp_assert(GetGenomeSize() == 0, "Cannot initialize VirtualCPUOrg twice");
      // Create the ancestor, either randomly or from a genome file
      if(SharedData().init_random) FillRandom(SharedData().init_length, random);
      else{
        Load(SharedData().initial_genome_filename);
        SharedData().init_length = GetGenomeSize();
      }
      // Set traits that are specific to the ancestor (others are in ResetTraits) 
      Organism::SetTrait<size_t>(SharedData().generation_name, 0); 
      Organism::SetTrait<double>(SharedData().merit_name, 
          GetGenomeSize() / SharedData().init_length); 
      // Call generic reset methods
      ResetHardware();
      ResetTraits();
    }

    /// Reset the organism back to starting conditions
    void Reset(){
      ResetHardware();
      ResetTraits();
    }
    
    /// Create an offspring organism using the configuration file's mutation rate.
    emp::Ptr<Organism> MakeOffspringOrganism(emp::Random & random) const {
      // Create and mutate
      auto offspring = OrgType::Clone().DynamicCast<VirtualCPUOrg>();
      const genome_t offspring_genome = 
          GetTrait<genome_t>(SharedData().offspring_genome_name);
      offspring->genome.resize(offspring_genome.size(), GetDefaultInst());
      std::copy(
          offspring_genome.begin(),
          offspring_genome.end(),
          offspring->genome.begin());
      offspring->ResetWorkingGenome();
      offspring->Mutate(random);
      offspring->Reset();
      double bonus = 0;
      if(SharedData().copy_influences_merit){
          bonus = std::min(
            {offspring->GetGenomeSize(), GetNumInstsCopied(), GetNumInstsExecuted()}
          );
      } 
      else bonus = std::min(offspring->GetGenomeSize(), GetNumInstsExecuted());
      bonus /= SharedData().init_length;
      // Initialize all necessary traits and ready hardware
      offspring->SetTrait<double>(SharedData().merit_name, 
          bonus + GetTrait<double>(SharedData().child_merit_name)); 
      offspring->SetTrait<double>(SharedData().child_merit_name, SharedData().initial_merit); 
      offspring->SetTrait<size_t>(SharedData().generation_name, 
          GetTrait<size_t>(SharedData().generation_name) + 1); 
      offspring->CurateNops();
      offspring->Organism::SetTrait<std::string>(
          SharedData().genome_name, offspring->GetGenomeString());
      offspring->Organism::SetTrait<size_t>(
          SharedData().genome_length_name, offspring->GetGenomeSize());
      offspring->Organism::GetTrait<emp::vector<data_t>>(SharedData().output_name).clear();
      offspring.DynamicCast<VirtualCPUOrg>()->ResetHardware();
      offspring.DynamicCast<VirtualCPUOrg>()->insts_speculatively_executed = 0;
      return offspring;
    }
    
    /// Create an identical organism with no mutations and with the same merit
    virtual emp::Ptr<Organism> CloneOrganism() const {
      auto offspring = OrgType::Clone().DynamicCast<VirtualCPUOrg>();
      offspring->ResetWorkingGenome();
      offspring->ResetHardware();
      offspring->SetTrait<double>(SharedData().merit_name, GetTrait<double>(SharedData().merit_name)); 
      offspring->SetTrait<double>(SharedData().child_merit_name, SharedData().initial_merit); 
      offspring->Organism::SetTrait<std::string>(SharedData().genome_name, offspring->GetGenomeString());
      offspring->Organism::SetTrait<size_t>(SharedData().genome_length_name, offspring->GetGenomeSize());
      offspring->Organism::GetTrait<emp::vector<data_t>>(SharedData().output_name).clear();
      offspring->expanded_nop_args = SharedData().expanded_nop_args;
      offspring->insts_speculatively_executed = 0;
      return offspring;
    }

    /// Load inputs and run the organism for a number of steps specified in the configuration
    /// file. Any generated outputs will be stored in the organism's output trait.
    void GenerateOutput() override {
      ResetHardware();

      // Setup the input.
      SetInputs(Organism::GetTrait<emp::vector<data_t>>(SharedData().input_name));

      // Run the code.
      //Process(SharedData().eval_time, SharedData().verbose);
    }

    /// Return a reference to the instruction library of the organism
    static inst_lib_t& GetInstLib(){
      static inst_lib_t inst_lib;
      return inst_lib;
    }

    /// Set up configuration options for this organism type
    void SetupConfig() override {
      GetManager().LinkVar(SharedData().point_mut_prob, "point_mut_prob",
                      "Per-site probability of a point mutation");
      GetManager().LinkVar(SharedData().insertion_mut_prob, "insertion_mut_prob",
                      "Per-site probability of a insertion mutation");
      GetManager().LinkVar(SharedData().deletion_mut_prob, "deletion_mut_prob",
                      "Per-site probability of a deletion mutation");
      GetManager().LinkFuns<size_t>([this](){ return GetGenomeSize(); },
                       [this](const size_t & /*N*/){ ClearGenome(); /*PushDefaultInst(N);*/ },
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
      GetManager().LinkVar(SharedData().position_name, "position_name",
                      "Where to store the organism's position?.");
      GetManager().LinkVar(SharedData().genome_length_name, "genome_length_name",
                      "Where to store the genome's length?.");
      GetManager().LinkVar(SharedData().merit_name, "merit_name",
                      "Name of variable corresponding to the organism's task performance.");
      GetManager().LinkVar(SharedData().child_merit_name, "child_merit_name",
                      "Name of variable corresponding to the organism's task performance that"
                      " will be used to calculate CPU cycles given to offspring.");
      GetManager().LinkVar(SharedData().generation_name, "generation_name",
                      "Name of variable corresponding to the organism's generation. "
                      "When an organism replicates, the child's gen. is the parent's gen +1");
      GetManager().LinkVar(SharedData().initial_merit, "initial_merit",
                      "Initial value for merit (task performance)");
      GetManager().LinkVar(SharedData().verbose, "verbose",
                      "If true, print execution info of organisms");
      GetManager().LinkVar(SharedData().inst_set_input_filename, "inst_set_input_filename",
                      "File that contains the instruction set to use."
                      " One instruction name per line. Order is maintained.");
      GetManager().LinkVar(SharedData().initial_genome_filename, "initial_genome_filename",
                      "File that contains the genome used to initialize organisms.");
      GetManager().LinkVar(SharedData().inst_set_output_filename, "inst_set_output_filename",
                      "If not empty, the instruction set is written to the specified file");
      GetManager().LinkVar(SharedData().expanded_nop_args, "expanded_nop_args",
                      "If true, some instructions (e.g., math) will use multiple nops to "
                      "fully define the registers used");
      GetManager().LinkVar(SharedData().use_speculative_execution, 
                      "use_speculative_execution",
                      "If true, we run as many instructions as possible and then cache the "
                      "results. Instructions that interact with the population or other "
                      "organisms will halt speculative execution.");
      GetManager().LinkVar(SharedData().max_speculative_insts, 
                      "max_speculative_insts",
                      "Maximum number of instructions to speculatively execute. "
                      "-1 for genome length.");
      GetManager().LinkVar(SharedData().copy_influences_merit, 
                      "copy_influences_merit",
                      "If 1, the number of instructions copied (e.g., via HCopy instruction)"
                      "factor into offspring merit");
    }

    /// Set up this organism type with the traits it need to track and initialize 
    /// shared variables.
    void SetupModule() override {
      SetupMutationDistribution();
      GetManager().AddOptionalTrait<emp::vector<data_t>>(SharedData().input_name);
      GetManager().AddSharedTrait(SharedData().output_name,
          "Value map output from organism.", emp::vector<data_t>());
      GetManager().AddSharedTrait<double>(SharedData().merit_name,
          "Value representing fitness of organism", SharedData().initial_merit);
      GetManager().AddSharedTrait<double>(SharedData().child_merit_name,
          "Fitness passed on to children", SharedData().initial_merit);
      GetManager().AddOwnedTrait<std::string>(SharedData().genome_name, 
          "Organism's genome", "[None]");
      GetManager().AddSharedTrait<genome_t>(SharedData().offspring_genome_name, 
          "Latest genome copied", { } );
      GetManager().AddSharedTrait<genome_t>("passed_genome", 
          "Genome as passed from parent", { } );
      GetManager().AddSharedTrait<OrgPosition>(SharedData().position_name, 
          "Organism's position ", { } );
      GetManager().AddOwnedTrait<size_t>(SharedData().generation_name, 
          "Organism's generation", 0);
      GetManager().AddOwnedTrait<size_t>(SharedData().genome_length_name, 
          "Length of organism's genome", 0);
      SetupInstLib();
      if(!SharedData().inst_set_output_filename.empty()){
        WriteInstructionSetFile(SharedData().inst_set_output_filename);
      }
    }

    /// Write the instructions in the instruction set (in order) to the specified file
    void WriteInstructionSetFile(const std::string& filename){
      std::cout << "Writing instruction set to file: " << filename << std::endl;
      emp::File file;
      std::stringstream ss;
      file << "index, id, char, name";
      for(size_t inst_offset = 0; inst_offset < GetInstLib().GetSize(); ++inst_offset){
        char inst_char = 'a' + inst_offset; 
        if(inst_offset > 25) inst_char = 'A' + (inst_offset - 26);
        size_t inst_id = GetInstLib().GetID(inst_char);
        size_t inst_idx = GetInstLib().GetIndex(inst_id);
        ss << inst_idx << ", " << inst_id << ", " << inst_char << ", " 
           << GetInstLib().GetName(inst_idx); 
        file << ss.str(); 
        ss.str("");
      }
      file.Write(filename);
    }

    /// Load from the file the instruction to use and what order to include them in
    emp::vector<std::string> LoadInstSetFromFile(){
      emp::File file(SharedData().inst_set_input_filename);
      file.RemoveComments("//");
      file.RemoveComments("#");
      file.RemoveWhitespace();
      file.RemoveEmpty();
      if(file.GetNumLines() == 0){
        emp_error("Error! VirtualCPUOrg instruction set file is either empty or missing: ", 
            SharedData().inst_set_input_filename);
      }
      return file.GetAllLines();
    }

    /// Load external instructions that were added via the configuration file
    void SetupInstLib(){
      inst_lib_t& inst_lib = GetInstLib();
      if(SharedData().use_speculative_execution) non_speculative_inst_vec.Clear();
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

      const emp::vector<std::string> name_vec = LoadInstSetFromFile();
      for(size_t inst_idx = 0; inst_idx < name_vec.size(); ++inst_idx){
        const std::string& name = name_vec[inst_idx];
        if(typed_action_map.find(name) == typed_action_map.end()){
          emp_error("Instruction '" + name + "' not found. Make sure the VirtualCPUOrg"
             " module comes after all instruction modules in the config file"); 
        }
        mabe::Action& action = typed_action_map[name];
        unsigned char c = 'a' + inst_idx;
        if(inst_idx > 25){
          c = 'A' + inst_idx - 26;
        }

        std::cout << "Found " << action.function_vec.size() << 
            " external functions with name: " << action.name << "!" <<
            " (" << c << ")" << std::endl;
        // If using speculative execution, see if this instruction breaks speculation
        if(SharedData().use_speculative_execution){
          // Ensure bitvec is large enough
          if(non_speculative_inst_vec.GetSize() < static_cast<size_t>(inst_idx + 1)){
            non_speculative_inst_vec.Resize(inst_idx + 1);
          }
          if(action.data.HasName("is_non_speculative") && 
              action.data.Get<bool>("is_non_speculative")){ // If non-speculative, flag it
            if(non_speculative_inst_vec.GetSize() < static_cast<size_t>(inst_idx + 1)){
              non_speculative_inst_vec.Resize(inst_idx + 1);
            }
            non_speculative_inst_vec[inst_idx] = true;
          } else{ // Assume instructions are okay with speculation by default
            non_speculative_inst_vec[inst_idx] = false;
          }
        }
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
            inst_idx);                          // Instruction ID
      }
    }

    /// Speculatively execute instructions up until an instruction modifies the outside world
    /// If instructions have already been speculatively executed, simply reduce their counter
    void Process_Speculative() {
      if(insts_speculatively_executed > 0){
        --insts_speculatively_executed;
      }
      else{
        const size_t max_insts = (SharedData().max_speculative_insts == -1)
            ? GetGenomeSize() : SharedData().max_speculative_insts;
        for(size_t offset = 0; offset < max_insts; ++offset){
          const size_t inst_id = genome_working[inst_ptr].id;
          if(!non_speculative_inst_vec[inst_id]){
            if(SharedData().verbose){
              std::cout << "[" << GetTrait<OrgPosition>(SharedData().position_name).Pos() 
                << "]" << std::endl;
            }
            Process(1, SharedData().verbose);
            ++insts_speculatively_executed; 
          }
          else{
            if(insts_speculatively_executed == 0){
              if(SharedData().verbose){
                std::cout << "[" << GetTrait<OrgPosition>(SharedData().position_name).Pos() 
                  << "]" << std::endl;
              }
              Process(1, SharedData().verbose);
            }
            else break;
            
          }
        }
      }
    }

    /// Process the next instruction, or use speculative execution if possible
    bool ProcessStep() override { 
      if(GetWorkingGenomeSize() == 0) return false;
      if(SharedData().use_speculative_execution){
        Process_Speculative();
      }
      else{
        if(SharedData().verbose){
          std::cout << "[" << GetTrait<OrgPosition>(SharedData().position_name).Pos()
            << "]" << std::endl;;
        }
        Process(1, SharedData().verbose);
      }
      return true;
    }

    /// Initialize the mutational distribution variables to match the genome size (either 
    /// current size or projected sizes)
    void SetupMutationDistribution(){
      if(GetGenomeSize() != 0){ // If we have a genome size, use it!
        SharedData().point_mut_dist.Setup(SharedData().point_mut_prob, GetGenomeSize());
        SharedData().insertion_mut_dist.Setup(SharedData().insertion_mut_prob, 
            GetGenomeSize());
        SharedData().deletion_mut_dist.Setup(SharedData().deletion_mut_prob, GetGenomeSize());
        SharedData().mut_sites.Resize(GetGenomeSize());
      }
      else{ // Otherwise, use the genome size set in the configuration file
        SharedData().point_mut_dist.Setup(SharedData().point_mut_prob, 
            SharedData().init_length);
        SharedData().insertion_mut_dist.Setup(SharedData().insertion_mut_prob, 
            SharedData().init_length);
        SharedData().deletion_mut_dist.Setup(SharedData().deletion_mut_prob, 
            SharedData().init_length);
        SharedData().mut_sites.Resize(SharedData().init_length);
      }
    }

  };

  MABE_REGISTER_ORG_TYPE(VirtualCPUOrg, "Organism consisting of Avida instructions.");
}

#endif
