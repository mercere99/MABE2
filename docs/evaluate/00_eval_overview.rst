
======================
What is an Evaluator?
======================

Evaluators are modules that measure how well organisms perform under a given circumstance.
They generally correspond to problems/fitness landscapes. 
When used on a population, they assign a trait to each organism indicating how well it performed.
In simple configurations, this trait may be used directly as a "fitness" criterion on which to base selection.

Using an evaluator
------------------

In addition to other configurable parameters that are specific to a given evaluator, all evaluators have a parameter called ``fitness_trait``.
This parameter indicates the name of the trait where the score calculated by the evaluator should be stored.

All evaluators have an ``EVAL`` member function. This function takes a list of organisms as an argument and runs the evaluation function on all of them, storing the results in the trait designated by the ``fitness_trait`` parameter.

For example, the NK evaluator can be used as follows::

    // Create a population
    Population main_pop;            // Main population for managing candidate solutions.

    // Configure an  EvalNK module with the name eval_nk
    EvalNK eval_nk {                // Evaluate bitstrings on an NK fitness lanscape.
        N = num_bits;               // Number of bits required in output
        K = 3;                      // Number of bits used in each gene
        bits_trait = "bits";        // Which trait stores the bit sequence to evaluate?
        fitness_trait = "fitness";  // Which trait should we store NK fitness in?
    };

    // Other modules would be configured here

    // Configure what happens on each time step
    @UPDATE(Var ud) {
        
        // Run the evaluator on the population
        eval_nk.EVAL(main_pop);
    }


Writing an evaluator
--------------------

Writing an evaluator module is the same as writing any other type of module, except for the following:

* The constructor must call ``SetEvaluateMod(true);`` to tell MABE that this is an evaluator module.

* The module must have a config parameter called ``fitness_trait`` that specifies where the calculated fitness should be stored.

* The module must have a member function named ``EVAL`` that takes a ``Collection`` as input, runs the evaluator on each member of the collection, and stores the result in ``fitness_trait``.

Example::

    // All modules should be in the MABE namespace
    namespace mabe {

    // All modules should inherit from Module
    class EvalExample : public Module {
    private:
        // The trait that fitness should be stored in
        std::string fitness_trait;

    public:
        EvalNK(mabe::MABE & control,
            const std::string & name="EvalNK",
            const std::string & desc="Module to evaluate bitstrings on an NK Fitness Lanscape",
            const std::string & _ftrait="fitness")
        : Module(control, name, desc)
        , fitness_trait(_ftrait)
        {
            // Notify MABE that this is an Evaluate module
            SetEvaluateMod(true);
        }
        ~EvalNK() { }

        // Setup member functions associated with this class.
        static void InitType(emplode::TypeInfo & info) {
            // Setup the EVAL function
            // The arguments are a reference to this module and a Collection of organisms
            // For convenience, this just calls a member function called Evaluate, which
            // we'll define a little later
            info.AddMemberFunction("EVAL",
                                [](EvalExample & mod, Collection list) { return mod.Evaluate(list); },
                                "Evaluate all orgs in an OrgList.");

            // Other functions you want the user to be able to call in the config file 
            // could be set up here
        }

        // Setup the config settings for this module
        // This needs to include fitness_trait, but could also 
        // include anything else you want the user to be able to configure
        void SetupConfig() override {
            // Our fitness_trait member variable will end up containing a string
            // (which by default is "fitness_trait") containing the name of the trait
            // where the fitness will be stored
            LinkVar(fitness_trait, "fitness_trait", "Which trait should we store fitness in?");
        
            // Other config parameter could be set up here
        }

        // Tell the module about any traits that it depends on
        void SetupModule() override {
            // Setup the traits.
            // This module creates the fitness_trait trait, which defaults to 0
            AddOwnedTrait<double>(fitness_trait, "Fitness value", 0.0);

            // Other setup could happen here
        }

        // Actually implement Evaluate method that we used earlier in the
        // EVAL function
        double Evaluate(const Collection & orgs) {
            // Loop through the population and evaluate each organism.

            // If you want to only evaluate orgs that are alive,
            // it may be helpful to create a new collection that
            // only contains the organisms within the original collection
            // that are actually alive.
            mabe::Collection alive_orgs( orgs.GetAlive() );

            // Keep track of the maximum score because
            // Evaluate needs to return it
            double max_score = 0.0;

            // Loop over all the organisms
            // (alive_orgs could be replaced with orgs if we didn't bother
            // with the filtering step above)
            for (Organism & org : alive_orgs) {
                // If you're going to look at any other traits to evaluate 
                // fitness, make sure to call GenerateOutput on the organism
                // to ensure all traits are populated
                org.GenerateOutput();

                // Figure out what you want this organism's fitness to be
                // (you probably want to do something more elaborate here)
                double fitness = 1;

                // Set this organisms fitness trait equal to the calculated fitness
                org.SetTrait<double>(fitness_trait, fitness);

                if (fitness > max_score) {
                    max_score = fitness;
                }
            }

            return max_score;
        }

        // Alternate version of Evaluate that takes a Population instead of a Collection
        // If a population is provided to Evaluate, first convert it to a Collection.
        double Evaluate(Population & pop) { return Evaluate( Collection(pop) ); }

        // Alternate version of Evaluate that takes a string instead of a Collection
        // If a string is provided to Evaluate, convert it to a Collection.
        double Evaluate(const std::string & in) { return Evaluate( control.ToCollection(in) ); }
    };

    // You always need to call MABE_REGISTER_MODULE after defining a new module,
    // to inform MABE that the module exists.
    // Remember to also add it in modules.hpp as an include.
    MABE_REGISTER_MODULE(EvalExample, "Example evaluator.");
    }