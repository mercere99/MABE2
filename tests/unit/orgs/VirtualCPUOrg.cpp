/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2022.
 *
 *  @file  VirtualCPUOrg.cpp
 *  @brief Test all functionality of the VirtualCPU organism. 
 */

// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// MABE
#include "core/EmptyOrganism.hpp"
#include "orgs/VirtualCPUOrg.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Nop.hpp"
#include "orgs/instructions/VirtualCPU_Inst_IO.hpp"

//
// TODO
//  [ ] SetupConfig
//    [ ] ???
//
//  Called in various other test cases
//  [X] SetupModule
//    [X] Manager mutation vars are set
//    [X] Traits are setup
//    [X] SetupInstLib was called
//  [X] SetupInstLib
//    [X] All expected instructions appear in inst. lib.
//  [ ] Test mutations
//    [X] Point
//    [ ] Insertion
//    [ ] Deletion

TEST_CASE("VirtualCPUOrg_Main", "[orgs]"){
  using org_t = mabe::VirtualCPUOrg;
  using mod_t = mabe::OrganismManager<org_t>;

  // Initialize the instruction library, which only needs done once
  mabe::MABE control;
  control.SetupEmpty<mabe::EmptyOrganismManager>();

  std::stringstream test_script;
  test_script <<
    "random_seed = 100;"
    "Population test_pop;"
    "VirtualCPU_Inst_Nop inst_nop;"
    "VirtualCPU_Inst_IO inst_io;"
    "VirtualCPUOrg vcpu_org { "
    "  point_mut_prob = 0.01;"
    "  insertion_mut_prob = 0.0;"
    "  deletion_mut_prob = 0.0;"
    "  inst_set_input_filename = 'inst_set_test.txt';"
    "};"
    ;
  control.Load(test_script, "Test script");
  control.Setup();

  // Now that the controller is setup, take some actions (normally in @START)
  control.Execute("test_pop.INJECT('vcpu_org', 2);");

  auto & org_module = dynamic_cast<mod_t&>(control.GetModule("vcpu_org"));
  auto & org_data = org_module.GetManagedData();

  mabe::Population & test_pop = control.GetPopulation("test_pop");
  CHECK(test_pop.GetSize() == 2);
  org_t & test_org = dynamic_cast<org_t&>(test_pop[0]);

  { // ManagerData
    // [X] Defaults correctly
    // [X] Can be changed
    // [X] Changes are seen by other orgs 

    // Check that all shared data default properly (other than mutation datastructs)
    CHECK(org_data.point_mut_prob == 0.01);
    CHECK(org_data.insertion_mut_prob == 0);
    CHECK(org_data.deletion_mut_prob == 0);
    CHECK(org_data.init_length == 100);
    CHECK(org_data.init_random == true);
    CHECK(org_data.eval_time == 500);
    CHECK(org_data.input_trait.GetName() == "input");
    CHECK(org_data.output_trait.GetName() == "output");
    CHECK(org_data.merit_trait.GetName() == "merit");
    CHECK(org_data.genome_trait.GetName() == "genome");
    CHECK(org_data.offspring_merit_trait.GetName() == "offspring_merit");
    CHECK(org_data.initial_merit == 0); 
    CHECK(org_data.verbose == false); 
    CHECK(org_data.initial_genome_filename == "ancestor.org");
    CHECK(org_data.expanded_nop_args == false);
    // Values can be changed
    test_org.SharedData().point_mut_prob = 0.05;
    CHECK(org_data.point_mut_prob == 0.05);
    test_org.SharedData().init_length = 1000;
    CHECK(org_data.init_length == 1000);
    test_org.SharedData().init_random = false;
    CHECK(org_data.init_random == false);
    test_org.SharedData().eval_time = 100;
    CHECK(org_data.eval_time == 100);
    test_org.SharedData().input_trait.SetName("input_2");
    CHECK(org_data.input_trait.GetName() == "input_2");
    test_org.SharedData().output_trait.SetName("output_2");
    CHECK(org_data.output_trait.GetName() == "output_2");
    test_org.SharedData().merit_trait.SetName("merit_2");
    CHECK(org_data.merit_trait.GetName() == "merit_2");
    test_org.SharedData().genome_trait.SetName("genome_2");
    CHECK(org_data.genome_trait.GetName() == "genome_2");
    test_org.SharedData().offspring_merit_trait.SetName("offspring_merit_2");
    CHECK(org_data.offspring_merit_trait.GetName() == "offspring_merit_2");
    test_org.SharedData().initial_merit = 1; 
    CHECK(org_data.initial_merit == 1); 
    test_org.SharedData().verbose = true; 
    CHECK(org_data.verbose == true); 
    test_org.SharedData().initial_genome_filename = "ancestor_expanded.org";
    CHECK(org_data.initial_genome_filename == "ancestor_expanded.org");
    test_org.SharedData().expanded_nop_args = true;
    CHECK(org_data.expanded_nop_args == true);

    // Value changes are reflected in organisms
    CHECK(org_data.point_mut_prob == 0.05);
    CHECK(org_data.init_length == 1000);
    CHECK(org_data.init_random == false);
    CHECK(org_data.eval_time == 100);
    CHECK(org_data.input_trait.GetName() == "input_2");
    CHECK(org_data.output_trait.GetName() == "output_2");
    CHECK(org_data.merit_trait.GetName() == "merit_2");
    CHECK(org_data.genome_trait.GetName() == "genome_2");
    CHECK(org_data.offspring_merit_trait.GetName() == "offspring_merit_2");
    CHECK(org_data.initial_merit == 1); 
    CHECK(org_data.verbose == true); 
    CHECK(org_data.initial_genome_filename == "ancestor_expanded.org");
    CHECK(org_data.expanded_nop_args == true);
  }

  // -------------------------------------------------------------------------------------
  { // Mutate
    //  [X] Mutate
    //    [X] Returned number of mutations matches changed positions
    //    [X] Mutation probability is used 
    //    [X] Genome trait is updated
 
    while(true){
      std::string original_genome = test_org.GetGenomeString();
      // Keep mutating organism until we get two mutations at once
      size_t mut_hits = test_org.Mutate(control.GetRandom()); 
      if (mut_hits == 2) { 
        std::cout << "Orig. genome:" << std::endl;
        std::cout << original_genome << std::endl;
        std::string new_genome = test_org.GetGenomeString();
        size_t realized_num_muts = 0;
        for (size_t idx = 5; idx < new_genome.size(); ++idx){
          if (original_genome[idx] != new_genome[idx]){
            realized_num_muts++;
          }
        }
        std::cout << "Mutated genome:" << std::endl;
        std::cout << new_genome << std::endl;
        CHECK(realized_num_muts == 2); // We should have two different instructions!
        break;
      }
    }

    // Ensure mutation probability is used
    std::string original_genome = test_org.GetGenomeString();
    org_data.point_mut_prob = 1.0;
    org_data.insertion_mut_prob = 0;
    org_data.deletion_mut_prob = 0;
    test_org.SetupMutationDistribution();
    CHECK(test_org.Mutate(control.GetRandom()) == 100);
    std::string new_genome = test_org.GetGenomeString();
    size_t realized_num_muts = 0;
    for(size_t idx = 5; idx < new_genome.size(); ++idx){
      if(original_genome[idx] != new_genome[idx]){
        realized_num_muts++;
      }
    }
    CHECK(realized_num_muts == 100);
  }
  { // Randomize
    control.SetRandomSeed(102);
    org_data.point_mut_prob = 0.01;
    test_org.SetupMutationDistribution();

    std::string original_genome = test_org.GetGenomeString();
    test_org.Randomize(control.GetRandom());
    std::string randomized_genome = test_org.GetGenomeString();
    size_t realized_num_muts = 0;
    for(size_t idx = 5; idx < randomized_genome.size(); ++idx){
      if(original_genome[idx] != randomized_genome[idx]){
        realized_num_muts++;
      }
    }
    CHECK(realized_num_muts >= 50); // Basic check to make sure its randomized, but 
                                        // some instructions may not actually change
  }

  // -------------------------------------------------------------------------------------
  { // Initialize
    //  [X] If init random is true, init randomly
    //  [X] Else, init to given genome
    //  [X] Genome trait is set
    //  [X] Merit trait is initialized
    //  [X] Child merit is initialized
    //  [X] Underlying VCPU is initialized
    //  [X] Underlying VCPU curates nops
    
    // Control initialization
    control.SetRandomSeed(103);
    org_data.init_length = 50;
    org_data.initial_merit = 20;;

    // Reset everything else to defaults...
    org_data.point_mut_prob = 0.01;
    org_data.insertion_mut_prob = 0;
    org_data.deletion_mut_prob = 0;
    org_data.init_random = true;
    org_data.eval_time = 500;
    org_data.input_trait.SetName("input");
    org_data.output_trait.SetName("output");
    org_data.merit_trait.SetName("merit");
    org_data.genome_trait.SetName("genome");
    org_data.offspring_merit_trait.SetName("offspring_merit");
    org_data.verbose = false;
    org_data.initial_genome_filename = "ancestor.org";
    org_data.expanded_nop_args = false;


    // Org initialization
    control.Execute("test_pop.INJECT('vcpu_org', 1);");
    CHECK(test_pop.GetSize() == 3);
    org_t & org = dynamic_cast<org_t&>(test_pop[2]);
    org.SetupMutationDistribution();

    CHECK(org.GetGenomeSize() == 50);
    CHECK(org.GetTrait<std::string>("genome") == org.GetGenomeString());
    CHECK(org.GetTrait<double>("merit") == 1.0);
    CHECK(org.GetTrait<double>("offspring_merit") == 20);
    CHECK(org.nops_need_curated == false);
    CHECK(org.are_nops_counted == true);


    // Org initialization
    control.Execute("test_pop.INJECT('vcpu_org', 1);");
    CHECK(test_pop.GetSize() == 4);

    org_t & org_2 = dynamic_cast<org_t&>(test_pop[3]);
    CHECK(org_2.GetGenomeSize() == 50);
    CHECK(org_2.GetTrait<std::string>("genome") == "[50]abcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcab");
    CHECK(org_2.GetTrait<double>("merit") == 1.0); 
    CHECK(org_2.GetTrait<double>("offspring_merit") == 20);
    CHECK(org_2.nops_need_curated == false);
    CHECK(org_2.are_nops_counted == true);
  }

  // -------------------------------------------------------------------------------------
  {// CloneOrganism 
    //  [X] All offspring traits match original traits
    //  [X] Offspring genome is _identical_
    control.SetRandomSeed(103);
    org_data.init_random = true;
    org_data.init_length = 50;

    // Org initialization
    control.Execute("test_pop.INJECT('vcpu_org', 1);");
    CHECK(test_pop.GetSize() == 5);
    org_t & org = dynamic_cast<org_t&>(test_pop[4]);
    org.SetupMutationDistribution();

    org.SetTrait<double>("merit", 2);
    org.SetTrait<double>("offspring_merit", 3);
    org.AdvanceIP(1);
    std::string original_genome = org.GetGenomeString();
    emp::Ptr<org_t> offspring_ptr = org.CloneOrganism().DynamicCast<org_t>();
    std::string offspring_genome = offspring_ptr->GetGenomeString();
    std::cout << "Parent: " << original_genome << std::endl;
    std::cout << "Child:  " << offspring_genome << std::endl;
    CHECK(original_genome == offspring_genome);
    CHECK(offspring_ptr->GetTrait<double>("merit") == org.GetTrait<double>("merit")); 
    CHECK(offspring_ptr->GetTrait<double>("offspring_merit") == org_data.initial_merit); 
    CHECK(offspring_ptr->inst_ptr == 0); 
    offspring_ptr.Delete();
  }
  { // MakeOffspringOrganism
    // [X] Offspring can be mutated
    // [X] Low mutation rate -> offspring might not be mutated
    // [X] Offspring traits are set
    //  [X] Parent's offspring merit -> offspring's merit
    control.SetRandomSeed(104);
    org_data.point_mut_prob = 0;

    // Org initialization
    control.Execute("test_pop.INJECT('vcpu_org', 1);");
    CHECK(test_pop.GetSize() == 6);
    org_t & org = dynamic_cast<org_t&>(test_pop[5]);
    org.SetupMutationDistribution();

    org.SetTrait<double>("merit", 2);
    org.SetTrait<double>("offspring_merit", 3);
    std::string original_genome = org.GetGenomeString();
    org.AdvanceIP(1);
    org.SetTrait<mabe::VirtualCPUOrg::genome_t>("offspring_genome", org.genome);
    emp::Ptr<mabe::VirtualCPUOrg> offspring_org_1 = 
        org.MakeOffspringOrganism(control.GetRandom()).DynamicCast<mabe::VirtualCPUOrg>();
    std::string offspring_genome_1 =
        offspring_org_1->GetGenomeString();
    std::cout << "Parent: " << original_genome << std::endl;
    std::cout << "Child:  " << offspring_genome_1 << std::endl;
    CHECK(original_genome == offspring_genome_1);
    // Merit should not have changed because we haven't copied any instructions
    CHECK(offspring_org_1->GetTrait<double>("merit") == org.GetTrait<double>("offspring_merit")); 
    CHECK(offspring_org_1->GetTrait<double>("offspring_merit") == org_data.initial_merit); 
    CHECK(offspring_org_1->inst_ptr == 0); 
    offspring_org_1.Delete();
    // Second offspring -> guaranteed mutations
    org_data.point_mut_prob = 1;
    org.SetupMutationDistribution();
    emp::Ptr<mabe::VirtualCPUOrg> offspring_org_2 = 
        org.MakeOffspringOrganism(control.GetRandom()).DynamicCast<mabe::VirtualCPUOrg>();
    std::string offspring_genome_2 =
        offspring_org_2->GetGenomeString();
    std::cout << "Parent: " << original_genome << std::endl;
    std::cout << "Child:  " << offspring_genome_2 << std::endl;
    CHECK(original_genome != offspring_genome_2);
    // Merit should not have changed because we haven't copied any instructions
    CHECK(offspring_org_2->GetTrait<double>("merit") == org.GetTrait<double>("offspring_merit")); 
    CHECK(offspring_org_2->GetTrait<double>("offspring_merit") == org_data.initial_merit); 
    CHECK(offspring_org_2->inst_ptr == 0); 
    offspring_org_2.Delete();
  }
  { // ProcessStep 
    control.SetRandomSeed(106);
    org_data.point_mut_prob = 0.01;
    org_data.init_random = false;
    org_data.initial_genome_filename = "org_nops.org";

    control.Execute("test_pop.INJECT('vcpu_org', 1);");
    CHECK(test_pop.GetSize() == 7);
    org_t & org = dynamic_cast<org_t&>(test_pop[6]);
    org.SetupMutationDistribution();

    CHECK(org.inst_ptr == 0);
    org.ProcessStep();
    CHECK(org.inst_ptr == 1);
    org.ProcessStep();
    CHECK(org.inst_ptr == 2);
  }
  /*
  { // GenerateOutput 
    control.GetRandom().ResetSeed(107);
    mabe::OrganismManager<mabe::VirtualCPUOrg> manager(control, "name", "desc");
    mabe::VirtualCPUOrg org(manager);
    org.SharedData().point_mut_prob = 0.01;
    org.SharedData().eval_time = 6;
    org.SharedData().init_random = false;
    org.SharedData().initial_genome_filename = "org_io_test.org";
    org.SharedData().verbose = true;
    org.SetupMutationDistribution();
    emp::DataMap data_map = control.GetOrganismDataMap();
    control.GetTraitManager().RegisterAll(data_map);
    data_map.LockLayout();          
    org.SetDataMap(data_map);
    org.CountNops();
    org.Initialize(control.GetRandom());
    org.GenerateOutput();
    emp::vector<uint32_t>& output_vec = org.GetTrait<emp::vector<uint32_t>>(
        org.SharedData().output_name);
    std::cout << org.SharedData().output_name << std::endl;
    for(size_t idx = 0; idx < output_vec.size(); ++idx){
      if(idx != 0) std::cout << ", ";
      std::cout << output_vec[idx];
    }
    CHECK(output_vec.size() == 6);
    CHECK(output_vec[0] == 0);
    CHECK(output_vec[1] == 1);
    CHECK(output_vec[2] == 2);
    CHECK(output_vec[3] == 2634983619);
    CHECK(output_vec[4] == 1250306466);
    CHECK(output_vec[5] == 2036905506);
  }
  */
}
