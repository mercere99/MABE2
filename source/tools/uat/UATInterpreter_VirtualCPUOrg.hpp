#include <iostream>
#include <fstream>
#include "UATInterpreter.hpp"
#include "orgs/VirtualCPUOrg.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Flow.hpp"
#include "orgs/instructions/VirtualCPU_Inst_IO.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Label.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Manipulation.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Math.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Nop.hpp"
#include "orgs/instructions/VirtualCPU_Inst_Replication.hpp"
#include "placement/AnnotatePlacement_Position.hpp"


namespace mabe{

  class UATInterpreter_VirtualCPUOrg : public uat::UATInterpreter<VirtualCPUOrg::data_t>{
  protected:

    // Initialize the instruction library, which only needs done once
    MABE control;
    emp::Ptr<OrganismManager<mabe::VirtualCPUOrg>> manager_ptr;
    emp::Ptr<VirtualCPUOrg> org_ptr;
    std::unordered_map<char, std::string> inst_set_map;

    void SpoofMABESetup(int random_seed = 0){
      control.GetRandom().ResetSeed(random_seed);
      control.AddPopulation("test_pop", 0);
      manager_ptr = 
          emp::NewPtr<mabe::OrganismManager<mabe::VirtualCPUOrg>>(control, "name", "desc");
      // Add instructions
      mabe::VirtualCPU_Inst_Flow         flow_inst_module(control);
      mabe::VirtualCPU_Inst_IO           io_inst_module(control);
      mabe::VirtualCPU_Inst_Label        label_inst_module(control);
      mabe::VirtualCPU_Inst_Manipulation manip_inst_module(control);
      mabe::VirtualCPU_Inst_Math         math_inst_module(control);
      mabe::VirtualCPU_Inst_Nop          nop_inst_module(control);
      mabe::VirtualCPU_Inst_Replication  replication_inst_module(control);
      // To appease the rep. inst. module
      mabe::AnnotatePlacement_Position annotate_module(control);
      mabe::VirtualCPUOrg tmp_org(*manager_ptr);
      control.GetTraitManager().Unlock();
      annotate_module.SetupModule();
      flow_inst_module.SetupModule();
      io_inst_module.SetupModule();
      label_inst_module.SetupModule();
      manip_inst_module.SetupModule();
      math_inst_module.SetupModule();
      nop_inst_module.SetupModule();
      replication_inst_module.SetupModule();
      tmp_org.SetupModule();
      control.GetTraitManager().Lock();
      control.GetTraitManager().Verify(true);
      std::cout << "InstLib size: " << tmp_org.GetInstLib().GetSize() << std::endl;
      emp::DataMap data_map = control.GetOrganismDataMap();
      control.GetTraitManager().RegisterAll(data_map);
      data_map.LockLayout();          
      org_ptr = emp::NewPtr<VirtualCPUOrg>(*manager_ptr);
    }

  public: 
    UATInterpreter_VirtualCPUOrg(const std::string& inst_set_filename, int random_seed = 0) 
        : UATInterpreter()
        , control(0, nullptr){
      LoadInstSet(inst_set_filename);
      SpoofMABESetup(random_seed);
    }
    UATInterpreter_VirtualCPUOrg(const std::string& inst_set_filename, const std::string& filename, int random_seed = 0) 
        : UATInterpreter(filename)
        , control(0, nullptr){
      LoadInstSet(inst_set_filename);
      SpoofMABESetup(random_seed);
    }
    UATInterpreter_VirtualCPUOrg(const std::string& inst_set_filename, std::istream& is, int random_seed = 0) 
        : UATInterpreter(is) 
        , control(0, nullptr){
      LoadInstSet(inst_set_filename);
      SpoofMABESetup(random_seed);
    }

    ~UATInterpreter_VirtualCPUOrg(){
      org_ptr.Delete();
      manager_ptr.Delete();
    }

    void LoadInstSet(const std::string& filename){
      emp::File file(filename);
      file.RemoveComments("//");
      file.RemoveComments("#");
      file.CompressWhitespace();
      for(size_t idx = 0; idx < file.GetNumLines(); ++idx){
        if(file[idx].size() > 0){
          char sym = 'a' + (char)idx;
          if(idx > 25) sym = 'A' + (char)(idx - 26); 
          emp::justify(file[idx]);
          inst_set_map[sym] = file[idx];
        }
      }

    }

    virtual void LoadGenome(const std::string& genome) override { 
      org_ptr->genome.clear();
      org_ptr->genome_working.clear();
      for(size_t idx = 0; idx < genome.size(); ++idx){
        org_ptr->PushInst(inst_set_map[genome[idx]]);
      }
      org_ptr->CurateNops();
      org_ptr->ResetHardware();
    } 
    virtual void SetReg(char reg_char, data_t val) override { 
      switch(reg_char){
        case 'a': 
          org_ptr->regs[0] = val;
          break;
        case 'b': 
          org_ptr->regs[1] = val;
          break;
        case 'c': 
          org_ptr->regs[2] = val;
          break;
        default:
          ThrowLineError("Invalid register character:" + reg_char);
      }
    }
    virtual void CheckReg(char reg_char, data_t expected_val) override { 
      data_t actual_val = 0;
      switch(reg_char){
        case 'a': 
          actual_val = org_ptr->regs[0]; 
          break;
        case 'b': 
          actual_val = org_ptr->regs[1];
          break;
        case 'c': 
          actual_val = org_ptr->regs[2];
          break;
        default:
          ThrowLineError("Invalid register character:" + reg_char);
      }
      if(actual_val == expected_val) successful_checks++;
      else{
        std::cout << emp::ANSI_BrightRed() << "CHECK_REG failed on line " << line_num << 
          "! Expected: " << expected_val << "; Actual: " << actual_val << "." << 
          emp::ANSI_Reset() << std::endl; 
        failed_checks++;
      }
    }
    virtual void Execute(size_t num_inst) override { 
      for(size_t i = 0; i < num_inst; ++i){
        org_ptr->ProcessStep();
      }
    }
    virtual void SetIP(data_t val) override { 
      org_ptr->SetIP(val);
    }
    virtual void SetRH(data_t val) override { 
      org_ptr->SetRH(val);
    }
    virtual void SetWH(data_t val) override { 
      org_ptr->SetWH(val);
    }
    virtual void SetFH(data_t val) override { 
      org_ptr->SetFH(val);
    }
    virtual void CheckIP(data_t expected_val) override { 
      data_t actual_val = org_ptr->inst_ptr;
      if(actual_val == expected_val) successful_checks++;
      else{
        std::cout << emp::ANSI_BrightRed() << "CHECK_IP failed on line " << line_num << 
          "! Expected: " << expected_val << "; Actual: " << actual_val << "." << 
          emp::ANSI_Reset() << std::endl; 
        failed_checks++;
      }
    }
    virtual void CheckRH(data_t expected_val) override { 
      data_t actual_val = org_ptr->read_head;
      if(actual_val == expected_val) successful_checks++;
      else{
        std::cout << emp::ANSI_BrightRed() << "CHECK_RH failed on line " << line_num << 
          "! Expected: " << expected_val << "; Actual: " << actual_val << "." << 
          emp::ANSI_Reset() << std::endl; 
        failed_checks++;
      }
    }
    virtual void CheckWH(data_t expected_val) override { 
      data_t actual_val = org_ptr->write_head;
      if(actual_val == expected_val) successful_checks++;
      else{
        std::cout << emp::ANSI_BrightRed() << "CHECK_WH failed on line " << line_num << 
          "! Expected: " << expected_val << "; Actual: " << actual_val << "." << 
          emp::ANSI_Reset() << std::endl; 
        failed_checks++;
      }
    }
    virtual void CheckFH(data_t expected_val) override { 
      data_t actual_val = org_ptr->flow_head;
      if(actual_val == expected_val) successful_checks++;
      else{
        std::cout << emp::ANSI_BrightRed() << "CHECK_FH failed on line " << line_num << 
          "! Expected: " << expected_val << "; Actual: " << actual_val << "." << 
          emp::ANSI_Reset() << std::endl; 
        failed_checks++;
      }
    }
    virtual void CheckStackA(size_t idx, data_t expected_val) override { 
      data_t actual_val = org_ptr->GetStackVal(0, idx);
      if(actual_val == expected_val) successful_checks++;
      else{
        std::cout << emp::ANSI_BrightRed() << "CHECK_STACK_A failed on line " << line_num << 
          "! Expected: " << expected_val << "; Actual: " << actual_val << "." << 
          emp::ANSI_Reset() << std::endl; 
        failed_checks++;
      }
    }
    virtual void CheckStackB(size_t idx, data_t expected_val) override { 
      data_t actual_val = org_ptr->GetStackVal(1, idx);
      if(actual_val == expected_val) successful_checks++;
      else{
        std::cout << emp::ANSI_BrightRed() << "CHECK_STACK_B failed on line " << line_num << 
          "! Expected: " << expected_val << "; Actual: " << actual_val << "." << 
          emp::ANSI_Reset() << std::endl; 
        failed_checks++;
      }
    }

  }; // End class

} // End namespace
