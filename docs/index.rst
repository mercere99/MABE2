.. MABE2 documentation master file, created by
   sphinx-quickstart on Fri Jul  2 11:15:02 2021.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. 
   Less of a technical page, more of a scicomm page. Should be written for a broad general audience. Why do you want to use MABE over other softwares? What advantages does it confer? What are its possible downsides? Why should you join us?

Welcome to MABE2's documentation!
=================================
.. image:: images/MABE.png
  :width: 600
  
`Visit MABE2 on github here <https://github.com/mercere99/MABE2>`_.

MABE is a software framework deigned to easily build and customize software for evolutionary computation or artificial life. 
The resulting systems should be useful for studying evolutionary dynamics, solving complex problems, comparing evolving systems, or exploring the open-ended power of evolution.

MABE version 2.0 is being re-built from scratch, using the Empirical library. 
Our goal is to allow for more modular control, flexible agents, faster run times and portability to the web.

.. toctree::
   :hidden:
   :maxdepth: 2
   :caption: First Steps
   :glob:

   first_steps/*

.. toctree::
   :hidden:
   :caption: Modules Overview
   :glob:

   modules/*


.. toctree::
   :hidden:
   :caption: Evaluators
   :glob:

   evaluate/*


.. toctree::
   :hidden:
   :caption: Organisms
   :glob:

   organisms/*