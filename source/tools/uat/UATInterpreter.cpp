#include "UATInterpreter_VirtualCPUOrg.hpp"

int main(int argc, char** argv){
  if(argc != 2){
    std::cout << "Error! Expected exactly one argument: the path of the uat file to run!" << std::endl;
    return 1;
  }
  mabe::UATInterpreter_VirtualCPUOrg interpreter("./instset-heads_wiki.cfg", std::string(argv[1]));
  interpreter.Process();
  interpreter.PrintResults();
  return 0;
}
