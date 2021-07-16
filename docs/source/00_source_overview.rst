==========
Overview of Source
==========

The files in the ``source`` directory an be divided into four different groups: CORE, MODULES, TOOLS and DEPENDENCIES. 
Below, we'll get into what each category means, and how items are sorted into each individual category. 
This page is meant to give an high-level overview of how the ``source`` folder is organized and define some key terms, as well as provide a resource
that can be used to understand some of the structure of ``MABE2``. 

MODULES
-------

Modules can be thought of as puzzle pieces - picking and choosing different modules to slot together allows experiments to be 
both customizable and easily modifiable. As an end user, you will spend a decent amount of time creating and using modules of all types. 
There are currently several different types of modules in ``MABE2``. Below is a list of each type, and where they are located inside of ``MABE2``. 
More in-depth information about each module can be found in the `Modules Page <../modules/00_module_overview.html>`_.

*  ``source/evaluate:`` Evaluator modules evaluate organisms in a run using any metrics.

*  ``source/placement:`` Placement modules organize where newborn offspring should be placed.

*  ``source/analyze:`` Analysis modules collect, process, and store population data.

*  ``source/orgs:`` Orgs (or Organisms) modules are the default organisms available for MABE runs.

* ``source/select:`` Select modules define how organisms are selected for the incoming population.

* ``source/interface:`` Interface modules provide a user interface for MABE.

* ``source/schema:`` Schema modules are extra Modules that control other aspects of MABE runs.


Tools for Analysis
------------------

Tools for Analysis (or just "Tools") are out-of-the-box-ready tools useful for data analysis. They can also be thought
of as extra tools to simplify building, running and analyzing MABE modules.
This group is periodically expanding as we work to put out more and more pre-made tools for end-users to 
use when processing their analysis. 

Currently, all of the tools can be found in the ``source/tools`` directory. 


CORE
----

Files and directories inside of the Core category are things that end users probably won't touch. 
These are the files and folders containing all of the core code required for any MABE run, and include files for the configuration scripting language.

All files in the Core category can be found in either ``source/core`` or ``source/config``.


DEPENDENCIES
------------

The files and directories in this category include libraries and other source code that ``MABE2`` uses to run. 
As an end user, you probably won't access these files. 

Currently, all files and directories in this category are in ``source/third-party``. 

Key Terms
---------------------

An ORGANSIM is a single agent in a MABE run.  A POPULATION is a collection of
organisms, which may all be defined individually.  A WORLD maintains one or
more populations of organisms an evaluation technique (its environment")

The dynamics of EVOLUTION are defined by a combination of an evaluation module
(i.e., a world/environment) for how their phenotype will be assessed, and a
selection module (i.e., a selection scheme) to determine how that phenotype will
influence and organism's ability to move on to the next generation.

Finally, DATA COLLECTION is defined by a set of analysis modules that monitor
a population throughout the evolution process.


Future Plans
-------------

Organisms should be defined by a controller module (i.e., a brain) to
handle how they process data, and a representation module (i.e., a genome) for
how that organism is stored and mutated.


