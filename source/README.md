The source directory is broken doen into seven folders, five for modules,
plus core functionality and tools to build new modules:

 represent/ - Modules that record and retrieve "genetic" informaton
 contol/    - Modules that define ontrollers/brains for individual organism

 evaluate/  - Modules that evaluate organisms in a run, using any metrics
 select/    - Modules that manage selection/reproduction mechanisms
 analyze/   - Modules that collect, process, and store population data

 core/      - All of the core code required for any MABE run
 tools/     - Extra tools to simplify building MABE modules

In general an ORGANISM is defined by a controller module (i.e., a brain) to
handle how they process data, and a representation module (i.e., a genome) for
how that organism is stored and mutatated.  A POPULATION is a collection of
organisms, which may all be defined individually.  A WORLD maintains one or
more populations of organisms an evalutation technique (its environment")

The dynamics of EVOLUTION are defined by a combination of an evaluation module
(i.e., a world/environment) for how their phenotype will be assessed, and a
selection module (i.e., a selection scheme) to determine how that phenotype will
influce and organism's ability to move on to the next generation.

Finally, DATA COLLECTION is defined by a set of analysis modules that monitor
a population throughout the evolution process.
