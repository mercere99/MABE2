#ifndef MABE_AVIDA_GP_ORGANISM_ILIB_H
#define MABE_AVIDA_GP_ORGANISM_ILIB_H

#include "hardware/AvidaGP.h"

namespace mabe{
  using hardware_t = emp::AvidaGP;
  using inst_t = hardware_t::inst_t;
  using genome_t = hardware_t::genome_t;
  using arg_t = hardware_t::arg_t;
  using inst_lib_t = emp::AvidaCPU_InstLib<hardware_t, arg_t, hardware_t::INST_ARGS>;

  static inst_lib_t base_inst_lib = inst_lib_t();//::DefaultInstLib();

  static void Inst_StartBirth(hardware_t & hw, const inst_t & inst) {
    hw.SetTrait(0, static_cast<size_t>(hw.GetTrait(0)) + 1);
    //std::cout << "Starting birth!" << std::endl;
  }
  static void Inst_Output_Trig(hardware_t & hw, const inst_t & inst) {
    // Save the date in the target reg to the specified output position.
    int output_id = (int) hw.regs[ inst.args[1] ];  // Grab ID from register.
    hw.outputs[output_id] = hw.regs[inst.args[0]];     // Copy target reg to appropriate output.
    hw.SetTrait(1, 1);
  }
  //static void Inst_Nand(hardware_t & hw, const inst_t & inst) {
  //    hw.regs[inst.args[2]] = ~(hw.regs[inst.args[0]] & hw.regs[inst.args[1]]);
  //}

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
      inst_lib.AddInst("Output", Inst_Output_Trig, 2,
				"Push reg Arg1 into output Arg2");
      inst_lib.AddInst("CopyVal", inst_lib_t::Inst_CopyVal, 2,
				"Copy reg Arg1 into reg Arg2");
      inst_lib.AddInst("ScopeReg", inst_lib_t::Inst_ScopeReg, 1,
				"Backup reg Arg1; restore at end of scope");
      inst_lib.AddInst("StartBirth", Inst_StartBirth, 0,
				"Begin replication");
      //inst_lib.AddInst("Nand", Inst_Nand, 3,
			//	"Perform the NAND logic operation");

      for (size_t i = 0; i < hardware_t::CPU_SIZE; i++) {
        inst_lib.AddArg(emp::to_string((int)i), i);                   // Args can be called by value
        inst_lib.AddArg(emp::to_string("Reg", 'A'+(char)i), i);  // ...or as a register.
      }
    }
    
    return inst_lib;
  }
}
#endif
