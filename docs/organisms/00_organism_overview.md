(_organisms)=

# What is an Organism?

Organisms are the entities that can inhabit populations. In an evolutionary computation context, they are what we would call candidate solutions (i.e. they are things that might solve our problem). In other contexts, you may see them referred to as agents, individuals, etc.

Organisms in MABE can reproduce. Their offspring will generally be similar to the parent, but on every reproduction event there is a chance of mutation.

Organism modules in MABE are responsible for specifying how organisms are encoded, any internal processing that they may do, and how they mutate.

## Add an organism type to MABE

Your configuration file should set up at least one organism module (e.g. `BitsOrg`, `ValsOrg`, `AvidaGPOrg`, etc.). At a minimum, most organism modules will require something akin to the following configuration parameters: 1) the name of a trait to store some representation of the organism in (often called `outout_name`), and 2) the mutation rate(s) or other information about how mutations should happen. In practice, many organism types will have a lot of other configuration parameters as well.

To add an pre-written organism type to main, initialize it in your config file like other modules. For example, to add the BitsOrg module, add the following to your configuration file:

```cpp
BitsOrg bits_org {      // Organism consisting of a series of N bits.
  output_name = "bits"; // Name of variable to contain bit sequence.
  N = num_bits;         // Number of bits in organism
  mut_prob = 0.01;	    // Probability of each bit mutating on reproduction.
};
```

## Genomes and brains

Sometimes it makes sense to draw a distinction between the genetic encoding for an organism (the genome) and the controller for the organism (the brain). The genome is the part of the organism that mutates. It encodes the brain and any other aspect of the organism.

Eventually, MABE will have genome and brain modules that can be combined flexibly to create a single organism.

Organisms can also be created directly, without using a genome or brain.

## Putting organisms in populations

Organisms live inside of `Population` objects. If desired, organisms of multiple types can be freely added to the same population (although this is an advanced use case and probably not what you usually want to do).

To add organisms to a population, you can use the `INJECT` action on a population in a config file:

```cpp 
// Declare a population
Population my_pop;

// Add the desired organism type to your configuration
BitsOrg bits_org {      // Organism consisting of a series of N bits.
  output_name = "bits"; // Name of variable to contain bit sequence.
  N = num_bits;         // Number of bits in organism
  mut_prob = 0.01;	    // Probability of each bit mutating on reproduction.
};

// Later, in one of the events, call INJECT
@START() {
    // Add 100 BitsOrg organisms to my_pop
    my_pop.INJECT(bits_org, 100);
}
```

(_write_org_module)=
## Writing an organism module

Setting up an organism module is a little different than setting up other types of modules. Read on to learn how!

### Class declaration

Instead of inheriting form the `Module` class,  it inherits from the `OrganismTemplate` class. One aspect that may be confusing is that this class needs to be templated off of the organism module class that your are currently writing. So the full declaration will look like `class MyOrg : public OrganismTemplate<MyOrg> {`. This is something called the curiously recursive template pattern (CRTP). The CRTP is a really interesting C++ technique that, for our purposes here, you do not need to understand. Just trust us that about how to declare the class :).

### Constructors

When you write an organism class, there are a few constructors you'll want to declare: 1) one that just takes an `OrganismManger` (templated off of your organism type), 2) copy constructor (can set to default), and 3) a constructor that takes a genome to initialize your organism based on.

```cpp

class MyOrg : public OrganismTemplate<MyOrg> {
  protected:
    // Your organism will probably have some sort of member variable
    // representing its genome. In this example its an int, but it
    // be whatever type you want. Just adjust the constructors 
    // accordingly
    int my_genome_val;

    // You can have other member variables here too if you want

  public:

    // Version of constructor that just takes an OrganismManager
    // templated off of your organism type
    MyOrg(OrganismManager<MyOrg> & _manager)
        : OrganismTemplate<MyOrg>(_manager),
        my_genome_val(0) // Initialize genome to 0 by default
        // You can initialize more member variables here if necessary
        { }

    // Copy constructor - use the default
    MyOrg(const MyOrg &) = default;
    MyOrg(MyOrg &&) = default;

    // Version of constructor that takes a genome as input
    // in addition to the organism manager
    MyOrg(int in, OrganismManager<MyOrg> & _manager)
        : OrganismTemplate<MyOrg>(_manager), 
          my_genome_val(in) { }

    // If you dynamically allocate any memory, remember to
    // deallocate it in the destructor
    ~MyOrg() { ; }

```

### Declare ManagerData

All organism modules need to declare an internal struct that inherits from the `Organism::ManagerData` class and holds all variables that are connected to configurable parameters:

```cpp
 struct ManagerData : public Organism::ManagerData {
      double mut_prob = 0.01;            ///< Probability of mutation
      std::string output_name = "my_trait";  ///< Name of trait that should be used to access genome
    };
```

When you want to access these variables elsewhere in the class, you will need to access them by calling `SharedData()`. For example, to access `mut_prob`, you would use `SharedData().mut_prob`.

### Required member functions

There are also a number of required member functions that are specific to organism modules

#### ToString

This function should return a string representation of your organism.

Example:

```cpp
// emp::to_string can convert most things (includes vectors, etc.) into strings
std::string ToString() const override { return emp::to_string(my_genome_val); }
```

#### Mutate

This function handles determining whether a mutation occurs on a given reproduction event, and applying that mutation if so. It takes a random number generator as an argument (since just about any mutation function you could write will need a random number generator). It should return the number of mutations that occurred.

```cpp
size_t Mutate(emp::Random & random) override {

    // Retrieve mutation probability from SharedData
    double p = SharedData().mut_prob;

    // random.P returns a 1 with with the given probability
    // and otherwise returns a 0
    if (random.P(p)) {
        // In this simplistic example, ever mutation increments
        // the genome value by 1. You probably want to do something
        // more interesting/elaborate
        my_genome_val++;

        // There was 1 mutation so return 1
        return 1;
    }

    // if we didn't mutate, return 0
    return 0;
}
```

#### Randomize

This function should set the organism to a random value (in whatever way makes the most sense for your organism). Takes a random number generator as input (MABE will pass in the main one MABE is using).

Example:

```cpp
void Randomize(emp::Random & random) override {
    // Sets the genome to a random integer
    my_genome_val = random.GetUInt();
}
```

#### Initialize

This function should initialize a new organism. It might call randomize, if you want to start with a random organism by default, or it might do something else.

Example:

```cpp
void Initialize(emp::Random & random) override {
    // In this example we'll just initialize it randomly
    Randomize(random);
}
```

#### GenerateOutput

This function makes sure that all traits the organism is supposed to store data in (probably the one holding the genome, at a minimum) are correctly populated.

Example:

```cpp
void GenerateOutput() override {
    // Store the genome's value in whatever trait is specified
    // in output_name (controlled via the config file)
    // Note that SetTrait is templated off the time of the
    // value you're storing in the trait
    SetTrait<int>(SharedData().output_name, my_genome_val);
}
```

#### SetupConfig

Like in other modules, SetupConfig is where you tell the configuration system what parameters the user should be able to configure about this type of organism. The one difference is that rather than being able to call `LinkVar` directly, you need to go through the ManagerData struct:

```cpp
/// Setup this organism type to be able to load from config.
void SetupConfig() override {
    GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                    "Probability of mutating on reproduction.");
    GetManager().LinkVar(SharedData().output_name, "output_name",
                    "Name of variable to contain genome.");
}

```

#### SetupModule

Similarly, SetupModule works like it does in other module types, except that you need to go through `SharedData()` and `GetManager()`. As with other modules, in SetupModule, you need to do any setup that should happen when the module is first intialized, and notify MABE of any trait requirements that the module has.

Example:

```cpp
/// Setup this organism type with the traits it need to track.
void SetupModule() override {

    // Setup the output trait.
    GetManager().AddSharedTrait(SharedData().output_name,
                                "Genome output from organism.",
                                0);
}
```

### Template organism module

Feel free to copy and modify this for your organism class!

```cpp
/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date year.
 *
 *  @file  FILE_NAME.hpp
 *  @brief Description here.
 */

#ifndef MABE_FILE_NAME_H
#define MABE_FILE_NAME_H

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"

namespace mabe {

  class MyOrg : public OrganismTemplate<MyOrg> {
  protected:

    // Replace this with whatever type you want
    int my_genome;

    // Add other private member variables here as necessary

  public:

    // Set your genome to whatever default you want
    MyOrg(OrganismManager<MyOrg> & _manager)
      : OrganismTemplate<MyOrg>(_manager), my_genome(0) { }

    // Change copy constructors if you need to do something
    // complicated (e.g. dynamically allocated memory)
    MyOrg(const MyOrg &) = default;
    MyOrg(MyOrg &&) = default;

    // Change int to the the type of your genome
    MyOrg(int in, OrganismManager<MyOrg> & _manager)
      : OrganismTemplate<MyOrg>(_manager), my_genome(in) { }

    // If you dynamically allocate memory, remember to delete it
    // in the destructor
    ~MyOrg() { ; }

    // Declare your ManagerData struct
    struct ManagerData : public Organism::ManagerData {
      double mut_prob = 0.01;            ///< Probability of mutating on reproduction.
      std::string output_name = "genome";  ///< Name of trait that should be used to access genome.
    };

    // Unless your genome is something really weird or you want
    // to add additional information, this should just work
    std::string ToString() const override { return emp::to_string(my_genome); }

    // Mutation function. Gets called on reproduction
    // Make this actually do what you want
    size_t Mutate(emp::Random & random) override {

        // Retrieve mutation probability from SharedData
        double p = SharedData().mut_prob;

        // random.P returns a 1 with with the given probability
        // and otherwise returns a 0
        if (random.P(p)) {
            // In this simplistic example, ever mutation increments
            // the genome value by 1. You probably want to do 
            // something more interesting/elaborate
            my_genome_val++;

            // There was 1 mutation so return 1
            return 1;
        }

        // if we didn't mutate, return 0
        return 0;
    }

    // Change this as appropriate for your genome
    // Should produce a random organism from within the
    // space of possible organisms
    void Randomize(emp::Random & random) override {
       my_genome_val = random.GetUInt();
    }

    // Change this as appropriate for your genome
    // Should set up an organism to be however you want
    // it to be by default (if not inheriting genetic
    // material from a parent)
    void Initialize(emp::Random & random) override {
      Randomize(random);
    }

    // Make sure your genome (and any other outputs) end up
    // in the correct traits
    void GenerateOutput() override {
        // Store the genome's value in whatever trait is specified
        // in output_name (controlled via the config file)
        // Note that SetTrait is templated off the time of the
        // value you're storing in the trait
        SetTrait<int>(SharedData().output_name, my_genome_val);
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
        GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                        "Probability of mutating on reproduction.");
        GetManager().LinkVar(SharedData().output_name, "output_name",
                        "Name of variable to contain genome.");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {

        // Setup the output trait.
        GetManager().AddSharedTrait(SharedData().output_name,
                                    "Genome output from organism.",
                                    0);
    }
  };

  // Special MABE_REGISTER command for organisms
  MABE_REGISTER_ORG_TYPE(MyOrg, "Example organism.");
}

// MAKE SURE TO ALSO ADD THIS FILE TO modules.hpp

#endif

```