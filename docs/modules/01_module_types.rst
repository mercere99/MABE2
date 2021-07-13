================
Types of Modules
================

Organisms
=========

An organism is a combination of a genome and a brain. Kind of.

Example
*******

Link to organism page.

Genomes
=======

A genome is a source of heritable and mutable data. Genomes usually provide 
data that is used to construct other elements of a module. For example, a genome 
may be used by a brain to determine that brain's structure or by a world to 
determine the properties of an agent's body. The genomes interface requires access
functions to write values into the genome (including randomizing the genome) and 
read values from the genome. Each type of genome defines its own internal data 
structures, access function behaviors, and mutation operators. 

Example
*******

Link to genome page.

Brains
======

Brains are data processors that receive input and deliver
output. Brains are the most common method bu which agents communicate with worlds
(although worlds interfacing directly with genomes is an option). Under this abstraction, 
genetic programs, artificial neural networks, Markov Brains, ~et cetera~ are classified 
as brains. Depending on the user's context, they may find it easier to think of brains as controllers, 
solvers, solutions, I?O machines, or even as chemical processes. The brian interface 
specifies how brains must define its internal workings, including update function, internal 
states, and internal data structures. 

Example
*******

Link to brain page.

Evaluators
==========

Evaluators are the functions on which organisms get tested. Evaluation modules are categorized by the types of IO they work with.

Static Evaluators
-----------------

Static evaluators do not take any input. They produce output as the solution to a single problem that does not change
(typical of most evolutionary computation systems). The two types of static output are ``BitVector`` (more information 
`here`_ ) and ``emp::vector<double>``.

.. _here: https://empirical.readthedocs.io/en/latest/library/bits/bits.html#bitvector

Example
*******

For an example of a static evaluator, visit the :ref:`NK evaluator page<../evaluate/EvalNK>`.

Value I/O
---------

These evaluation modules provide a set of doubles (``emp::vector<double>`` or
``std::unordered_set<size_t,double>``) as input, and expect a new vector of
doubles back as an output.

In some cases, these are used for choosing a move in a game, where the output
vector lists all possible moves and the highest value of a legal move is the
one that is automatically chosen.  Such a system allows the organism to simply
weight each possible move without needing to actually select one.

Example
*******

At the current moment there are no value I/O evaluators. 

Selectors
=========

Selectors manage populations, select parents, oversee re- production, and terminate agents who get to be too old or fail to meet certain criteria. 
Roulette selection, tournament selection, lambda+n, lexicase selection, and MAP-Elites are all examples of the types of algorithms optimizers 
typically implement.

Example
*******

Link to selector page.

Placement
=========

Example
*******

Link to placement page.

Schema
======

Example
*******

Link to schema page.