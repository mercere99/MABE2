#ifndef MABE_ANALYZE_SYSTEMATICS_MODULE_H
#define MABE_ANALYZE_SYSTEMATICS_MODULE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"
#include "../core/TraitSet.hpp"

namespace mabe {

class AnalyzeSystematics : public Module {
private:
    bool store_outside;

public:
    AnalyzeSystematics(mabe::MABE & control,
               const std::string & name="AnalyzeSystematics",
               const std::string & desc="Module to track the population's phylogeny.",
               bool _storeout = 0)
      : Module(control, name, desc), store_outside(_storeout)
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
};

    MABE_REGISTER_MODULE(AnalyzeSystematics, "Module to track the population's phylogeny.");
};
#endif
