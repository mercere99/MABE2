The source directory is broken doen into seven folders, five for modules,
plus core functionality and tools to build new modules:

 core/      - All of the core code required for any MABE run
 tools/     - Extra tools to simplify building MABE modules
 config/    - Configuration scripting language.

 orgs/      - Default organisms available for MABE runs

 evaluate/  - Modules that evaluate organisms in a run, using any metrics.
 select/    - Modules that manage selection/reproduction mechanisms.
 placement/ - Modules that organize where newborn offspring should be placed.
 analyze/   - Modules that collect, process, and store population data.
 interface/ - Modules thar provide a user interface for MABE.
 schema/    - Extra Modules that control other aspects of MABE runs.

An ORGANSIM is a single agent in a MABE run.  A POPULATION is a collection of
organisms, which may all be defined individually.  A WORLD maintains one or
more populations of organisms an evalutation technique (its environment")

The dynamics of EVOLUTION are defined by a combination of an evaluation module
(i.e., a world/environment) for how their phenotype will be assessed, and a
selection module (i.e., a selection scheme) to determine how that phenotype will
influce and organism's ability to move on to the next generation.

Finally, DATA COLLECTION is defined by a set of analysis modules that monitor
a population throughout the evolution process.

Plans: Organisms should be defined by a controller module (i.e., a brain) to
handle how they process data, and a representation module (i.e., a genome) for
how that organism is stored and mutatated.
