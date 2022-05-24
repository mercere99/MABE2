# Quickstart

## Installation

For more detail, see the [intallation and compilation](00_installation) instructions. For most people, though, running the following instructions in a terminal should be sufficient:

```shell
git clone --recurse-submodules https://github.com/mercere99/MABE2.git
cd MABE2/build
make
```

## Running MABE

In order to run, MABE needs a configuration file telling it how to set up the evolutionary scenario you want to study. By convention, MABE configuration files use the `.mabe` file extension.

To quickly get familiar with how MABE works, the easiest thing to do is run it with a pre-written configuration file. For this example, we will use the one that comes with MABE in `settings/NK.mabe`. This configuration file evolves bitstrings (i.e. sequence of 1s and 0s) on an [NK landscape](../evaluate/EvalNK). 

You can tell MABE which configuration file to use with the `-f` flag. Thus, the full command to run mabe with the NK example file is:

```shell
cd build
./MABE -f ../settings/NK.mabe
```

Ta-da! You have now used MABE! This example will run for 1000 generations and print some data on fitness in the population as it goes. It will also print out some data files containing more information.

## Writing your own configuration file

Using pre-existing configuration files is all well and good, but to really use MABE for research purposes you will need to be able to write your own configuration files.

Your configuration file needs to set up two main things: 1) variables/modules, and 2) signals.

### Setting up variables/modules

Variables can be declared anywhere in MABE configuration files and used to hold multiple types of objects, modules, and values. Specifically, variables may have the following types:

- Generic variables (`Var`): Hold arbitrary numbers or strings
- Populations (`Population`): Hold populations of organisms
- Lists of organisms (`OrgList`): Hold lists of organisms (which may be a subset of the organisms in a whole population or may contain organisms from multiple populations)
- Random seed: `random_seed` is a special value that is automatically declared. Thus, you do not need to specify a type for it. Instead, you can just set it to whatever number you choose. This will control the seed given to the random number generator that MABE uses whenever it needs a random number.
- Data files (`DataFile`): Contain instructions for printing data to a files.
- Modules: Modules are initialized in MABE configuration files in the same way that other objects are. Modules are the core components of MABE - they are used to encapsulate the different components of a computational evolution system that can be plugged together.

These variables can be initialized at any point in the configuration file (including within signal responses) by listing the type and name of the variable. When the variable require configuration information (as with modules or data files), this configuration information is specified in curly braces after the variable name. For example:

```cpp

random_seed = 0; // Set the random seed to 0 (note that 0 is a special value that tells the random number generator to pick a seed based on the time)

Var my_number = 10;  // Creates a generic variable called my_number that contains the value 10

Population my_population; // Creates a population called my_population. Populations can be manipulated by calling methods and functions.

OrgList my_orgs; // Creates an empty OrgList called my_orgs. This list can be populated by calling other functions and storing the results in it

DataFile my_file { filename="my_file.csv"; }; // Creates a DataFile that will store data in the file my_file.csv. To tell the datafile what to write, you will need to call methods on it. To tell the datafile when to write, you will need to call its WRITE() method in a signal handler

ExampleModule my_module { // Creates an ExampleModule called my_module
    example_trait = "trait"; // Sets the module's example_trait parameter to "trait"
    example_number = 3; // Sets the module's example_number parameter to 3
}; // Note: ExampleModule is not a real module - this will not work in real configuration file
```

At a minimum, most configurations will need the following:

- A population for your organisms to live in. If you are using synchronous/non-overlapping generations, you will need two populations, one for the current generation and one for the next.
- An organism module, to specify what type of organisms you are using.
- An evaluation module, to specify how fitness is assigned to your organisms
- A selection module, to specify how organisms are chosen to reproduce
- For clarity, we recommend setting the random seed explicitly, even if you are setting it to 0 (i.e. based on time)
- Assuming you have a fixed population size, we recommend storing it in a variable for convenience when setting up signals (see below)

Here is an example of how to do those minimal steps for a configuration using synchronous generations:

```cpp
random_seed = 0;          // Base random seed on time.
Var pop_size = 500;       // Set population size to 500
Population main_pop;      // Main population
Population next_pop;      // Offspring population

// The organism module. In this case, we are using BitsOrg, in which
// organisms are all bitstrings (series of 1s and 0s).
// to use a different type of organism, replace `BitsOrg` with something
// else. The organism stores its bit sequence (i.e. its "genome") in the
// trait specified by the output_name parameter
BitsOrg my_org_module {
  output_name = "bits";   // Name of trait to contain bit sequence.
  N = 10;           // Number of bits in organism. Here we're using 10.
  mut_prob = 0.01;  // Probability of each bit mutating on reproduction.
};

// The evaluation module. In this case, we are using EvalCountBits, which
// assigns fitness based on the number of 1s or 0s in the organism. To use
// a different type of evaluator, replace `EvalCountBits` with something 
// else. The bits_trait parameter tells this module where to look for
// the organism's bitstring. Thus, we need to make sure it matches the
// output_name parameter in the organism module. 
EvalCountBits my_evaluator {
  bits_trait = "bits";  // Which trait stores the bit sequence to evaluate?
  score_trait = "fitness";  // Which trait should we store fitness in?
  count_type = 1; // Indicates that we should count 1s, not 0s.
};


// The selection module. In this case, we are using SelectTournament, which
// selects organisms to reproduce using Tournament Selection (a set of 
// individuals are randomly selected and the fittest gets to reproduce).
// To use a different kind of selection, replace `SelectTournament` with 
// something else. The fitness_fun parameter tells this module where to
// look to find each organism's fitness. Thus, we need to make sure it 
// matches the score_trait parameter in the evaluation module.
SelectTournament my_selector {
  tournament_size = 7;       // Number of orgs in each tournament
  fitness_fun = "fitness";   // Which trait provides the fitness value?
};
```

### Setting up signals

There are various points in the running of MABE when you may want to make certain things happen. The two most important are at the very beginning of a run of MABE (`START`) and at the beginning of each new time step (`UPDATE`). To configure what happens at each of these times, you can write a simple function in the MABE configuration language.

The syntax for writing this code is the `@` sign, followed by the name of the event you are writing code for, followed by parentheses containing any function arguments that are available for that event, followed by curly braces containing your code. For example:

```cpp
@START() { // START does not take any arguments
    // All code here will run on the start event
}

// UPDATE takes one argument - a number 
// indicating the current time step
@UPDATE(Var ud) { 
    // All code here will run at the beginning of each time step
}
```

The code that you can write for these events is very flexible. In most cases, you will probably want to do the following:

On start:

- Add organisms to the population (can be done using the INJECT method of populations)
- Optionally, print a message containing configuration information

On update:

- Check whether you have reached the final update and stop if so (to avoid an infinite loop)
- Run your evaluator module on your population
- Run your selection module on your population
- If your generations are synchronous, swap your offspring population into your main population
- Optionally, print information about the progress of your run

Here is a minimal configuration that accomplishes those goals for a synchronous population (this is designed to be the second half of the configuration above):

```cpp
@START() {
  // Optional: print welcome message and population size
  PRINT("Starting MABE! Population size = ", population_size, "\n");  

  // Initialize population by adding organisms of the type specified
  // by the organism module we declared earlier and named "my_org_module".
  // The second argument to INJECT indicates how many organisms to add.
  // By adding pop_size, we ensure that the population starts out full.
  main_pop.INJECT("my_org_module", pop_size);
}

@UPDATE(Var ud) {

  // Check whether this is the 1000th update and stop MABE if it is
  IF (ud == 1000) EXIT();

  // Run evaluator on the main population
  my_evaluator.EVAL(main_pop);

  // Optional: print out some stats
  // This prints the current update, the size of the population,
  // and the average fitness in the population.
  // Note that this has to happen after we run EVAL, because
  // that's what assigns fitness to each organism
  PRINT("UD:", GET_UPDATE(),
        "  MainPopSize=", main_pop.SIZE(),
        "  AveFitness=", main_pop.CALC_MEAN("fitness"),
       );

  // Select organisms from main_pop, put their offspring in
  // next_pop, and repeat this [pop_size] times to fill up
  // the entire next population
  my_selector.SELECT(main_pop, next_pop, pop_size);

  // Swap the population of offspring into the main population
  main_pop.REPLACE_WITH(next_pop);
}

```

### Starter configuration file

For convenience, here is the whole vanilla configuration file described above in one place:

```cpp
random_seed = 0;          // Base random seed on time.
Var pop_size = 500;       // Set population size to 500
Population main_pop;      // Main population
Population next_pop;      // Offspring population

// The organism module. In this case, we are using BitsOrg, in which
// organisms are all bitstrings (series of 1s and 0s).
// to use a different type of organism, replace `BitsOrg` with something
// else. The organism stores its bit sequence (i.e. its "genome") in the
// trait specified by the output_name parameter
BitsOrg my_org_module {
  output_name = "bits";   // Name of trait to contain bit sequence.
  N = 10;           // Number of bits in organism. Here we're using 10.
  mut_prob = 0.01;  // Probability of each bit mutating on reproduction.
};

// The evaluation module. In this case, we are using EvalCountBits, which
// assigns fitness based on the number of 1s or 0s in the organism. To use
// a different type of evaluator, replace `EvalCountBits` with something 
// else. The bits_trait parameter tells this module where to look for
// the organism's bitstring. Thus, we need to make sure it matches the
// output_name parameter in the organism module. 
EvalCountBits my_evaluator {
  bits_trait = "bits";  // Which trait stores the bit sequence to evaluate?
  score_trait = "fitness";  // Which trait should we store fitness in?
  count_type = 1; // Indicates that we should count 1s, not 0s.
};


// The selection module. In this case, we are using SelectTournament, which
// selects organisms to reproduce using Tournament Selection (a set of 
// individuals are randomly selected and the fittest gets to reproduce).
// To use a different kind of selection, replace `SelectTournament` with 
// something else. The fitness_fun parameter tells this module where to
// look to find each organism's fitness. Thus, we need to make sure it 
// matches the score_trait parameter in the evaluation module.
SelectTournament my_selector {
  tournament_size = 7;       // Number of orgs in each tournament
  fitness_fun = "fitness";   // Which trait provides the fitness value?
};

@START() {
  // Optional: print welcome message and population size
  PRINT("Starting MABE! Population size = ", population_size, "\n");  

  // Initialize population by adding organisms of the type specified
  // by the organism module we declared earlier and named "my_org_module".
  // The second argument to INJECT indicates how many organisms to add.
  // By adding pop_size, we ensure that the population starts out full.
  main_pop.INJECT("my_org_module", pop_size);
}

@UPDATE(Var ud) {

  // Check whether this is the 1000th update and stop MABE if it is
  IF (ud == 1000) EXIT();

  // Run evaluator on the main population
  my_evaluator.EVAL(main_pop);

  // Optional: print out some stats
  // This prints the current update, the size of the population,
  // and the average fitness in the population.
  // Note that this has to happen after we run EVAL, because
  // that's what assigns fitness to each organism
  PRINT("UD:", GET_UPDATE(),
        "  MainPopSize=", main_pop.SIZE(),
        "  AveFitness=", main_pop.CALC_MEAN("fitness"),
       );

  // Select organisms from main_pop, put their offspring in
  // next_pop, and repeat this [pop_size] times to fill up
  // the entire next population
  my_selector.SELECT(main_pop, next_pop, pop_size);

  // Swap the population of offspring into the main population
  main_pop.REPLACE_WITH(next_pop);

```

### Using DataFiles

If you're trying to use MABE to do science, you probably want to output some data to a file. MABE has a very flexible system for configuring what data you want to output. To use a DataFile, you need to do the following:

#### Declare the data file

```cpp
DataFile my_file { filename="my_file.csv"; };
```

#### Configure the data file

You can configure the data file by calling methods on it. You can do this at any point in the configuration file after the the data file has been declared (although you probably do not want to do it inside the code for an event). The two main methods to know about are `ADD_COLUMN`, which adds a column to the data table being stored in your file, and `ADD_SETUP`, which adds a function that should be run for each row of the table before calculating the column values.

For example:

```cpp
// Add a column called Average Fitness where the value for each row is
// calculated by calling the function main_pop.CALC_MEAN('fitness'), 
// which will calculate the mean value of the fitness trait among the
// organisms in the population main_pop.
my_file.ADD_COLUMN( "Average Fitness", "main_pop.CALC_MEAN('fitness')" );

```

Or as an example that uses `ADD_SETUP`:

```cpp
// Create a variable called best_org that will hold
// a list of the best organism in the population
OrgList best_org;

// Tell the file to run this line of code before calculating the
// data for a given row. This will find the organism in main_pop
// that has the highest fitness and store it in the variable best_org
my_file.ADD_SETUP("best_org = main_pop.FIND_MAX('fitness')");

// Add a column called Best Genome where the value is calculated
// by accessing the `bits` trait of the organism stored in
// best_org. Note that this only works because we added the setup
// function, which will ensure that best_org is always correctly
// populated before we perform this calculation. 
my_file.ADD_COLUMN( "Best Genome", "best_org.TRAIT('bits')" );
```

#### Tell the data file when to output rows

Finally, you need to tell the data file when to calculate and output new rows. You can tell the data file to calculate and output a single new row with the `WRITE` method. Most commonly, you will want to this every update (or perhaps every so many updates), which can be achieved by calling `WRITE` from inside the `@UPDATE` event code:

```cpp
@UPDATE(Var ud) {
    // Do stuff here

    // This tells the file to calculate and write a new row
    my_file.WRITE();

    // Do more stuff here
}
```

Or if you wanted to only write data every 10 updates, you could do:

```cpp
@UPDATE(Var ud) {
    // Do stuff here

    // Check whether this is a time step on which we want to output
    // before we write a row
    IF (ud % 10 == 0) {
        my_file.WRITE();
    }
    // Do more stuff here
}
```

## Writing your own module

If MABE already has all the modules you need, feel free to skip this section! However, more likely than not, there will be some piece of additional functionality that you need (perhaps because it is something new that you are designing!). In that case, you will need to write a new module. MABE is designed with the goal of making it easy to write small, well-encapsulated modules that will plug-and-play with each other.

Since MABE is all written in C++, modules need to be written in C++ too. However, we have tried hard to protect users from having to write any particularly complicated C++ code (although that comes at the cost of MABE's internals being fairly complex - don't be intimidated if you don't understand all of the code in the core directory).

Note: The following information applies to writing any type of module other than organism modules. If you want to write an organism module, see {ref}`_write_org_module`.


Each module is its own C++ class. All modules must inherit from the Module base class. Besides that, the only thing required of modules is that they contain the following four member functions (described in more detail below):

- A constructor that tells MABE what kind of module it is
- SetupConfig()
- SetupModule()
- InitType()

Optionally, modules can also contain member functions that indicate things that should happen when specific events occur in MABE:

- `BeforeUpdate` - Occurs when current time step (update) is ending and new one is about to start. Arguments: Update ID that is just finishing.
- `OnUpdate` - Occurs when new time step (update) has just started. Arguments: Update ID just starting. NOTE: This is only for events where the order does not matter. If the timing of your event response matters relative to the timing of what other modules do, you should not use OnUpdate. Instead, you should use InitType to provide a method for the user to call so they can control what order things happen in.
- `BeforeRepro` - An organism is about to reproduce. Arguments: Position of organism about to reproduce.
- `OnOffspringReady` - Offspring is ready to be placed. Arguments: Offspring to be born, position of parent, population to place offspring in.
- `OnInjectReady` - Organism to be injected (i.e. placed into the population without a parent) is ready to be placed. Arguments: Organism to be injected, population to inject into.
- `BeforePlacement` - Placement location has been identified (For birth or inject). Arguments: Organism to be placed, placement position, parent position (if available)
- `OnPlacement` - New organism has been placed in the population. Arguments: Position new organism was placed.
- `BeforeMutate` - Mutate is about to run on an organism. Arguments: Organism about to mutate.
- `OnMutate` - Organism has had its genome changed due to mutation. Arguments: Organism that just mutated.
- `BeforeDeath` - Organism is about to die. Arguments: Position of organism about to die.
- `BeforeSwap` - Two organisms' positions in the population are about to move. Arguments: Positions of organisms about to be swapped.
- `OnSwap` - Two organisms' positions in the population have just swapped. Arguments: Positions of organisms just swapped.
- `BeforePopResize` - Full population is about to be resized. Arguments: Population about to be resized, the size it will become.
- `OnPopResize` - Full population has just been resized. Arguments: Population just resized, previous size it was.
- `BeforeExit` - Run immediately before MABE is about to exit. No arguments.
- `OnHelp` - Run when the --help option is called at startup. No arguments.

Any member function with one of these names will be run when the specified event occurs.

You are free to create additional helper functions to assist in writing these functions.

### Constructor

All the body of the constructor needs to do is call one of the following functions to indicate what type of module this is:

- `SetAnalyzeMod(true)`
- `SetEvaluateMod(true)`
- `SetInterfaceMod(true)`
- `SetManageMod(true)`
- `SetMutateMod(true)`
- `SetPlacementMod(true)`
- `SetSelectMod(true)`
- `SetVisualizeMod(true)`

As arguments, the constructor should take 1) a reference to a `mabe::MABE` object, 2) a string indicting the module's name, 3) a string describing the module, and 4) all other parameters needed to initialize the module, with sensible defaults. The first three arguments should be passed to the constructor for the `Module` base class.

For example, the constructor might look something like this:

```cpp
MyModule(mabe::MABE & control,
       const std::string & name="MyModule",
       const std::string & desc="A description",
       size_t _some_numeric_parameter=100, 
       const std::string & _some_string_parameter="a string")
  : Module(control, name, desc)
  , some_numeric_parameter(_some_numeric_parameter),
  , some_string_parameter(_some_string_parameter)
{
  SetEvaluateMod(true);
}
```

Note: this assumes the class has member variables called `some_numeric_parameter` and `some_string_parameter`.

### SetupConfig

The SetupConfig member function sets up this module's user-configurable parameters. This is accomplished by calling the following functions:

- `LinkVar`: links a member variable to a configuration parameter. The first argument is the variable to store the configured value in, the second is the name of the value in the config file, and the third is a description of the setting

For example, here is the SetupConfig function from the `SelectElite` module:

```cpp
void SetupConfig() override {
  LinkVar(fit_equation, "fitness_fun", "Function used as fitness for selection?");
  LinkVar(top_count, "top_count", "Number of top-fitness orgs to be replicated");
}
```

This creates two config options called "fitness_fun" and "top_count", linked to the member variables `fit_equation` and `top_count` respectively.

### SetupModule

The SetupModule member function is where the module indicates what traits it creates and what traits it requires other modules to have created. Recall that traits are the primary way that MABE modules communicate with each other and store information.

You can use the following functions here:

- `AddPrivateTrait` - Add trait that this module can READ & WRITE this trait Others cannot use it. Must provide name, description, and a default value to start at.
- `AddOwnedTrait` - Add trait that this module can READ & WRITE to; other modules can only read. Must provide name, description, and a default value to start at.
- `AddGeneratedTrait` - Add trait that this module can READ & WRITE to; at least one other module MUST read it. Must provide name, description, and a default value to start at.
- `AddSharedTrait` - Add trait that this module can READ & WRITE; other modules can too.
- `AddRequiredTrait` - Add trait that this module can READ, but another module must WRITE to it. That other module should also provide the description for the trait.
- `AddRequiredEquation` - Add all of the traits that that this module needs to be able to READ, in order to compute the provided equation. Another module must WRITE these traits and provide the descriptions.

You can also do any other set-up that the module requires here.

For example, here is the SetupModule function from the `EvalNK` module:

```cpp
void SetupModule() override {
  // Setup the traits.

  // Some other module needs to create a trait named the same thing
  // as the string in the `bits_trait` variable. That trait should contain
  // an object of type emp::BitVector.
  AddRequiredTrait<emp::BitVector>(bits_trait);

  // This module is creating a trait named whatever the variable `fitness_trait`
  // contains. It holds a double, which starts at 0.0 as a default, and its 
  // description is "NK fitness value"
  AddOwnedTrait<double>(fitness_trait, "NK fitness value", 0.0);

  // Do other necessary set-up
  landscape.Config(N, K, control.GetRandom());
}
```

### InitType

This function configures what methods the user is allowed to call on this module in the configuration file. It takes a reference to a `emplode::TypeInfo` object as input. Most of what you need to do in this function is to call the `AddMemberFunction` member function of that object, which allows you to specify the names, code, and descriptions for each user-callable function.

If you are writing an Evaluation module, you must add a function called EVAL that does the evaluation. If you are writing a Selection module you must add a function called SELECT that does the selection. Beyond that, you are free to add whatever functions make sense.

For readability, it often makes sense to put the bulk of the code for these functions into helper functions that are called from the function provided here.

For example, here is the InitType function from the `EvalNK` module:

```cpp
static void InitType(emplode::TypeInfo & info) {
 
  // Add the required EVAL method, which says to just call this module's
  // Evaluate() helper function
  info.AddMemberFunction("EVAL",
                         [](EvalNK & mod, Collection list) { return mod.Evaluate(list); },
                         "Use NK landscape to evaluate all orgs in an OrgList.");
  
  // Add another method called RESET that resets the fitness landscape
  info.AddMemberFunction("RESET",
                         [](EvalNK & mod) { mod.landscape.Config(mod.N, mod.K, mod.control.GetRandom()); return 0; },
                         "Regenerate the NK landscape with current N and K.");
}
```


### Making your new module available for configuration

- Include the macro to setup the module (`MABE_REGISTER_MODULE(MyModule, "My description.");`). This should be placed after your class definition.
- Add the file to modules.h
- Place the module in an example .gen and .mabe pair of files.

### Extras needed for official modules

The rules above all assume that you are trying to build a MABE module that will be functional for your own needs and for you to share with others. If you are trying to build a module that you intend to be shipped with the core MABE distribution, there are a few other things that you will need to do.

1. The heading at the top of your file must be done in doxygen style with a release under the MIT Software license and an @brief description of the module.

2. The include guard should begin with MABE_* 

3. The module should be placed in the "mabe" namespace.


### Template module

Feel free to copy this and adapt it as necessary for your own module.

```cpp
/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2022.
 *
 *  @file  [File_Name_Here].hpp
 *  @brief Describe the file
 */

#ifndef MABE_FILE_NAME_HERE_H
#define MABE_FILE_NAME_HERE_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

// If you want your module to become an official part
// of MABE, it needs to be in the MABE namespace
namespace mabe {

  // By convention, the actual name of your module should 
  // start with Eval, Select, or Analyze depending on what
  // type of module it is. If it as organism, it should end 
  // with Org.
  class MyModule : public Module {
  private:
    // A string containing the name of a trait that this
    // module needs to work with. Nearly all modules will
    // need to interact with at least one trait, although
    // exceptions are possible.
    std::string example_trait;

    // Any other member variables you need go here

  public:
    // Constructor for your module
    // The first three arguments to the constructor are the 
    // same for all modules, because they need to be passed
    // to the constructor for the Module class. Just 
    // customize the name and description to reflect the
    // details of your module.
    //
    // Add any additional arguments that are necessary to
    // initialize your member variables. Here, for example,
    // we added _example_trait, which we use to set the value
    // of example_trait. You should include reasonable 
    // defaults for all of these arguments.
    // 
    // In general, most values that you would set up to
    // control by parameters should probably be arguments
    // to your constructor
    MyModule(mabe::MABE & control,  
                  const std::string & name="MyModule",
                  const std::string & desc="A description of your module should go here.",
                  const std::string & _example_trait="my_trait_name"
                  )
      : Module(control, name, desc) // Pass args to base class constructor
      , example_trait(_example_trait) // Initialize example_trait
    {
      // The only thing that needs to happen in the constructor
      // is that you need to call a function to tell MABE what
      // kind of module this is. In this case we're setting
      // this to be an Evaluate mod.

      SetEvaluateMod(true);
    }

    // The default destructor should be fine for most modules
    // but if you dynamically allocate any memory make sure
    // to deallocate it here
    ~MyModule() { }

    // Select and Evaluate modules need an InitType member
    // function. It should take a reference to an
    // emplode::TypeInfo object as an argument. Other
    // module types can optionally have an InitType if they
    // want to allow the user to run methods from the config
    // file.
    //
    // This function is where all methods of this module
    // that are callable from the config file are defined.
    // At a minimum, Evaluate modules need an EVAL function
    // and Selection modules need a SELECT function, but
    // you can define others to.
    //
    // For an evaluate module, you can safely copy and paste
    // the below code verbatim into your module (although
    // ideally you should write an actual description), 
    // assuming you define an Evaluate method for this 
    // module (that's where you provide the actual code
    // for evaluating an organism's fitness)
    static void InitType(emplode::TypeInfo & info) {
      // Tell the configuration system that: 
      // - this module has a method called EVAL
      // - when EVAL is called, the lambda function in the
      //   second argument should be run.
      // - It should use the third argument as a description
      //   of this method
      info.AddMemberFunction("EVAL",
                             [](MyModule & mod, Collection list) { return mod.Evaluate(list); },
                             "Description of EVAL.");

    // If you are writing a select module, the commented out
    // code below is an example of what you could use to add
    // a select method (you will need to define a Select method
    // for this module that actually defines how 
    // selection works)

    //   info.AddMemberFunction("SELECT",
    //                          [](SelectElite & mod, 
    //                             Population & from, 
    //                             Population & to, 
    //                             double count) 
    //                         {
    //                           return mod.Select(from,to,count);
    //                         },
    //     "Perform elite selection on the provided organisms.");         
    }

    // SetupConfig is where you tell MABE what config options
    // this module should have available.
    void SetupConfig() override {
      // Call LinkVar for each config option you want to add
      // The first argument should contain a member variable
      // where you want to store the value the user sets.
      // The second argument is the default value and the
      // third is the description for this parameter.
      // Here, we are linking our example_trait member variable
      // to this parameter, and calling the parameter "my_trait_name".
      LinkVar(example_trait, "my_trait_name", "Description here");

      // Other necessary set-up for this module can happen here
    }

    // SetupModule is where you tell MABE about this module's
    // trait needs. Remember, traits are values stored on
    // organisms, and are the primary way modules communicate
    // within MABE. Traits can be either private (only usable
    // by this module), owned (only writeable by this module
    // but readable by others), shared (all modules can read
    // and write), or required (another module should generate
    // it, but this one needs to be able to read it).
    void SetupModule() override {
      // Tell MABE that another module needs to have created a trait
      // called whatever value is stored in example_trait, that
      // this module needs to be able to read that trait, and that
      // it should contain an int.
      AddRequiredTrait<int>(example_trait);

      // Tell MABE that this module is creating a trait called 
      // another_trait (note: you usually shouldn't hard-code trait
      // names like this - instead you should store them in a variable
      // as with example_trait), and that other modules are allowed to
      // read that trait but not write to it. It's description is 
      // "Another example", it's type is double, and its default value
      // is 0.0.
      AddOwnedTrait<double>("another_trait", "Another example", 0.0);
    }

    // For an evaluate module, this is the function that does most of
    // the work. It takes a Collection of organisms as input, and it
    // must somehow end up assigning 
    // 
    // Note: You only need this for evaluation modules!
    double Evaluate(Collection orgs) {

      // Evaluate needs to return the maximum score,
      // so we need to keep track of it
      double max_score = 0.0;

      // Loop through the population and evaluate each organism.
      // You may want to select the subset of organisms that are alive
      mabe::Collection alive_collect( orgs.GetAlive() );
      for (Organism & org : alive_collect) {        
        // Make sure this organism has its traits ready to access.
        org.GenerateOutput();

        // Do stuff here to figure out this organism's fitness
        // and store it in the variable fitness
        double fitness = 1;

        // Store fitness in the example trait
        // Note: if you're writing a real evaluator, you should
        // call the trait you store fitness in either
        // fitness_trait or score_trait, for consistency.
        // Here we're storing it in example_trait because that's
        // all we have
        org.SetTrait<double>(example_trait, fitness);
                
        // Keep track of maximum observed fitness
        if (fitness > max_score) {
            max_score = fitness;
        }
      }

      // Evaluate needs to return the maximum observed fitness
      return max_score;
    }

    // If we were instead writing a Select module, we would need a
    // Select function. Here is a template. 
    //
    // Note: you only need this for selection modules!!!
    //
    // Select takes references to two populations (the first is the 
    // one to select from and the second is the one to put offspring
    // in) and a number indicating how many rounds of selection to do.
    Collection Select(Population & select_pop, Population & birth_pop, size_t num_births) {
    
      // Grab a reference to the main random number generator for MABE
      // so that we can genrate a random number
      emp::Random & random = control.GetRandom();

      // Select needs to return a Collection containing all newly added
      // organisms. Initialize an empty collection to contain them
      Collection placement_list;

      // Loop through each round of selection.
      for (size_t round = 0; round < num_births; round++) {
        
        // Choose a random organism
        size_t best_id = random.GetUInt(N);
        while (select_pop[best_id].IsEmpty()) best_id = random.GetUInt

        // Code to actually do desired selection method goes here

        // Replicate the winner organism
        placement_list += control.Replicate(select_pop.IteratorAt(best_id), birth_pop, 1);
      }

      return placement_list;
    }

    // Functions associated with events 
    
    // THESE ARE ALL OPTIONAL!!!
    // Any that you aren't using should be removed

    // Format:  BeforeUpdate(size_t update_ending)
    // Trigger: Update is ending; new one is about to start
    // Args:    Update ID that is just finishing.
    void BeforeUpdate(size_t update_ending) override {

    }

    // Format:  OnUpdate(size_t new_update)
    // Trigger: New update has just started.
    // Args:    Update ID just starting.
    void OnUpdate(size_t new_update) override {
        // Before you write on OnUpdate member function, think carefully
        // about what you are doing in it. Is it just something that has
        // to happen regularly, or is it something that could interact
        // with other modules? If it's the latter, strongly consider making
        // it a method (initialized in InitType), to give the user control
        // over the order of events.
    }

    // Format:  BeforeRepro(OrgPosition parent_pos) 
    // Trigger: Parent is about to reproduce.
    // Args:    Position of organism about to reproduce.
    void BeforeRepro(OrgPosition parent_pos) override {

    }

    // Format:  OnOffspringReady(Organism & offspring, OrgPosition parent_pos, Population & target_pop)
    // Trigger: Offspring is ready to be placed.
    // Args:    Offspring to be born, position of parent, population to place offspring in.
    void OnOffspringReady(Organism & offspring, OrgPosition parent_pos, Population & target_pop) override {

    }

    // Format:  OnInjectReady(Organism & inject_org, Population & target_pop)
    // Trigger: Organism to be injected is ready to be placed.
    // Args:    Organism to be injected, population to inject into.
    void OnInjectReady(Organism & inject_org, Population & target_pop) override {

    }

    // Format:  BeforePlacement(Organism & org, OrgPosition target_pos, OrgPosition parent_pos)
    // Trigger: Placement location has been identified (For birth or inject)
    // Args:    Organism to be placed, placement position, parent position (if available)
    void BeforePlacement(Organism & org, OrgPosition target_pos, OrgPosition parent_pos) override {

    }

    // Format:  OnPlacement(OrgPosition placement_pos)
    // Trigger: New organism has been placed in the population.
    // Args:    Position new organism was placed.
    void OnPlacement(OrgPosition placement_pos) override {

    }

    // Format:  BeforeMutate(Organism & org)
    // Trigger: Mutate is about to run on an organism.
    // Args:    Organism about to mutate.
    void BeforeMutate(Organism & org) override {

    }

    // Format:  OnMutate(Organism & org)
    // Trigger: Organism has had its genome changed due to mutation.
    // Args:    Organism that just mutated.
    void OnMutate(Organism & org) override {

    }

    // Format:  BeforeDeath(OrgPosition remove_pos)
    // Trigger: Organism is about to die.
    // Args:    Position of organism about to die.
    void BeforeDeath(OrgPosition remove_pos) override {

    }

    // Format:  BeforeSwap(OrgPosition pos1, OrgPosition pos2)
    // Trigger: Two organisms' positions in the population are about to move.
    // Args:    Positions of organisms about to be swapped.
    void BeforeSwap(OrgPosition pos1, OrgPosition pos2) override {

    }

    // Format:  OnSwap(OrgPosition pos1, OrgPosition pos2)
    // Trigger: Two organisms' positions in the population have just swapped.
    // Args:    Positions of organisms just swapped.
    void OnSwap(OrgPosition pos1, OrgPosition pos2) override {

    }

    // Format:  BeforePopResize(Population & pop, size_t new_size)
    // Trigger: Full population is about to be resized.
    // Args:    Population about to be resized, the size it will become.
    void BeforePopResize(Population & pop, size_t new_size) override {

    }

    // Format:  OnPopResize(Population & pop, size_t old_size)
    // Trigger: Full population has just been resized.
    // Args:    Population just resized, previous size it was.
    void OnPopResize(Population &, pop size_t old_size) override {

    }

    // Format:  BeforeExit()
    // Trigger: Run immediately before MABE is about to exit.
    void BeforeExit() override {

    }

    // Format:  OnHelp()
    // Trigger: Run when the --help option is called at startup.
    void OnHelp() override {

    }

    // Any other helper functions that your module needs go here

  };

  // We always need to call MABE_REGISTER_MODULE to notify MABE 
  // about the new module
  // Remember to also add it in modules.hpp as an include.
  MABE_REGISTER_MODULE(MyModule, "My description.");
}

#endif


```