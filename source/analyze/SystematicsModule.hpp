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
      
    void OnUpdate(size_t /*update*/) override {
      sys.Update();
      // Systematic manager should check whether it's population is synchronous
    }

    void BeforeDeath(OrgPosition pos) override {
      sys.RemoveOrg({pos.Pos(), pos.PopID()});
    }

    void BeforePlacement(Organism& org, OrgPosition pos, OrgPosition ppos) override {
      if (ppos.IsValid()) {
        sys.AddOrg(org, {pos.Pos(), pos.PopID()}, {ppos.Pos(), ppos.PopID()});
      } else {
        // We're injecting so no parent
        // Double-check that this is happening because pop is null,
        // not because parent position is illegal
        // emp_assert(ppos.PopPtr().IsNull() && "Illegal parent position");
        sys.AddOrg(org, {pos.Pos(), pos.PopID()}, nullptr);
      }
    }

    void OnSwap(OrgPosition pos1, OrgPosition pos2) override {
      sys.SwapPositions({pos1.Pos(), pos1.PopID()}, {pos2.Pos(), pos2.PopID()});
    }
};

    MABE_REGISTER_MODULE(AnalyzeSystematics, "Module to track the population's phylogeny.");
};
#endif
