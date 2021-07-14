================
Types of Modules
================

Organisms
=========

For more detailed information on organisms, see the `organisms page <../organisms/traitinfo.html>`_ .

An organism is a combination of a genome and a brain. Kind of.

Genomes
-------

For detailed information on genomes, see **genome page link**.

A genome is a source of heritable and mutable data. Genomes usually provide 
data that is used to construct other elements of a module. For example, a genome 
may be used by a brain to determine that brain's structure or by a world to 
determine the properties of an agent's body. The genomes interface requires access
functions to write values into the genome (including randomizing the genome) and 
read values from the genome. Each type of genome defines its own internal data 
structures, access function behaviors, and mutation operators. 

Brains
------

For detailed information on brains, see **brain page link**.

Brains are data processors that receive input and deliver
output. Brains are the most common method bu which agents communicate with worlds
(although worlds interfacing directly with genomes is an option). Under this abstraction, 
genetic programs, artificial neural networks, Markov Brains, ~et cetera~ are classified 
as brains. Depending on the user's context, they may find it easier to think of brains as controllers, 
solvers, solutions, I?O machines, or even as chemical processes. The brian interface 
specifies how brains must define its internal workings, including update function, internal 
states, and internal data structures. 

Evaluators
==========

For detailed information on genomes, see the `evaluator page <../evaluate/EvalPacking.html>`_ .

Evaluators are the functions on which organisms get tested. Evaluation modules are categorized by the types of IO they work with.

Static Evaluators
-----------------

Static evaluators do not take any input. They produce output as the solution to a single problem that does not change
(typical of most evolutionary computation systems). The two types of static output are ``BitVector`` (more information 
`here`_ ) and ``emp::vector<double>``.

.. _here: https://empirical.readthedocs.io/en/latest/library/bits/bits.html#bitvector

Value I/O
---------

These evaluation modules provide a set of doubles (``emp::vector<double>`` or
``std::unordered_set<size_t,double>``) as input, and expect a new vector of
doubles back as an output.

In some cases, these are used for choosing a move in a game, where the output
vector lists all possible moves and the highest value of a legal move is the
one that is automatically chosen.  Such a system allows the organism to simply
weight each possible move without needing to actually select one.

At the current moment there are no static evaluators. 

Selectors
=========

For detailed information on selectors, see **selectors page link**.

Selectors manage populations, select parents, oversee re- production, and terminate agents who get to be too old or fail to meet certain criteria. 
Roulette selection, tournament selection, lambda+n, lexicase selection, and MAP-Elites are all examples of the types of algorithms optimizers 
typically implement.

Placement
=========

For detailed information on placement, see **placement page link**.

Placement modules dictate what happens when a new organism is placed into a population. 
This applies to both injected organisms, as happens at the start of the run, and offspring organisms.  

Schema
======

For detailed information on schema, see **schema page link**.

Schema modules are those which don't fit elsewhere. This includes modules such as ``MovePopulation``, which dictates how organisms
are shifter from one population to another, and ``Mutate``, which allows for extra mutations in a population.

Analyze
=======

Interface
=========