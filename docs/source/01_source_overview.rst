==========
Overview of Source
==========

The files in the `source` directory an be divided into four different groups: CORE, MODULES, TOOLS and DEPENDENCIES. 
Below, we'll get into what each category means, and how items are sorted into each individual category. 
This page is meant to give an high-level overview of how the ``source`` folder is organized, as well as provide a resource
that can be used to understand some of the structure of ``MABE2``. 

MODULES
-------

Modules can be thought of as puzzle pieces - picking and choosing different modules to slot together allows experiments to be 
both customizable and easily modifiable. As an end user, you will spend a decent amount of time creating and using modules of all types. 
There are currently several different types of modules in ``MABE2``. Below is a list of each type, and where they are located inside of ``MABE2``. 
More in-depth information about each module can be found in the `Modules Page <../modules/00_module_overview.html>`_.

*  **Evaluate:** Evaluator files, or ways to evaluate the fitness of an organism, can be found in ``MABE2/source/evaluate``.

*  **Placement:** Placement files, which decide how new organisms are placed into a population, can be found in ``MABE2/source/placement``.

*  **Analyze:** Analysis files can be found in ``MABE2/source/analyze``.

*  **Orgs:** Orgs (or Organisms) files, which define different types of organsims, can be found in ``MABE2/source/orgs``.

* **Select:** Select files, which define how organisms are selected for the incoming population, are located in ``MABE/source/select``.

* **Interface:** Interface files can be found in ``MABE2/source/interface``.

* **Schema:** Schema files are additional modules that don't fit nicely anywhere else. They're found in ``MABE2/source/schema``.


Tools for Analysis
------------------

Tools for Analysis (or just "Tools") are out-of-the-box-ready tools useful for data analysis. 
This group is periodically expanding as we work to put out more and more pre-made tools for end-users to 
use when processing their analysis. 

Currently, all of the tools can be found in the ``MABE2/source/tools`` directory. 


CORE
----

Files and directories inside of the Core category are things that end users probably won't touch. 
These are files and directories that make up the core functionality of ``MABE2``.

All files in the Core category can be found in either ``MABE2/source/core`` or ``MABE2/source/config``.


DEPENDENCIES
------------

The files and directories in this category include libraries and other source code that ``MABE2`` uses to run. 
As an end user, you probably won't access these files. 

Currently, all files and directories in this category are in ``MABE2/source/third-party``. 


