#ifndef MABE_ANALYZE_SYSTEMATICS_MODULE_H
#define MABE_ANALYZE_SYSTEMATICS_MODULE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"
#include "../core/TraitSet.hpp"
#include "emp/Evolve/Systematics.hpp"


namespace mabe {

class AnalyzeSystematics : public Module {
private:
    bool store_outside;
    emp::Systematics <Organism, int> sys;

public:
    AnalyzeSystematics(mabe::MABE & control,
               const std::string & name="AnalyzeSystematics",
               const std::string & desc="Module to track the population's phylogeny.",
               bool _storeout = 0)
      : Module(control, name, desc), store_outside(_storeout), sys([](Organism& org){return 1;})
    {
      SetAnalyzeMod(true);    ///< Mark this module as an analyze module.
    }
    ~AnalyzeSystematics() { }

    void SetupConfig() override {
      LinkVar(store_outside, "store_outside", "Store all taxa that ever existed.(1 = TRUE)" );
    }

    void SetupModule() override {
      // Setup the traits.
      //AddPrivateTrait<bool>(store_outside, "Store all taxa", 0);
    }
      
    void OnUpdate(size_t update) override{
      sys.Update();
      // Systematic manager should check whether it's population is synchronous
    }
    void BeforeDeath(OrgPosition pos){
      sys.RemoveOrg({0,0});
      // Change RemoveOrg to be and org position
    }
    void BeforePlacement(Organism& org, OrgPosition pos, OrgPosition ppos){
      //sys.AddOrg(org, pos, ppos);}
            sys.AddOrg(org);
    }
};

    MABE_REGISTER_MODULE(AnalyzeSystematics, "Module to track the population's phylogeny.");
};
#endif
