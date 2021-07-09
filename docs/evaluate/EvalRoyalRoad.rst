==========
Royal Road
==========

Overview
--------

The EvalRoyalRoad module evaluates organisms based on their ability to sequentially place "bricks" of ones. A "brick" is 
completed once a set of n ones are grouped together. "Bricks" are placed from the beginning of the organism to the end. 
A brick can only be placed immediately succeeding another brick. 

Traits
------

For a description of each type of trait, see traits documentation that doesn't exist yet.


+----------------+-------------------+--------------------------------------------+
| Type of Trait  | Trait Name        | Trait Description                          |
+================+===================+============================================+
|  Private       |    N/A            |                                            |
+----------------+-------------------+--------------------------------------------+
|  Owned         | ``fitness_trait`` |  Stores the score of each organism.        |
+----------------+-------------------+--------------------------------------------+
|  Generated     |    N/A            |                                            |
+----------------+-------------------+--------------------------------------------+
|  Shared        |    N/A            |                                            |
+----------------+-------------------+--------------------------------------------+
|  Required      | ``bits_trait``    |  Stores the bit sequence to be evaluated.  |
+----------------+-------------------+--------------------------------------------+
|  Optional      |    N/A            |                                            |
+----------------+-------------------+--------------------------------------------+
  
Parameters
----------
* ``brick_size`` the number of ones necessary to make a full brick.

Parameters may be modified in the file ``RoyalRoad.mabe``. 

I/O
---

Input
*****

None.

Output
******

None.

Compiling and Running
-------

To run MABE with the RoyalRoad evaluator, navigate to the build directory. 
First run the command ``make``. 
Next run the command ``./MABE -f settings/EvalRoyalRoad.mabe``.

Examples
-------
Here are a couple of example ``BitOrgs`` and how they would be evaluated by the RoyalRoad evaluator. 

+-------------------------------+-------------------+-----------------------------+
| Organism                      | ``brick_size``    | Fitness Score               |
+===============================+===================+=============================+
|    111000111                  |    3              |     1                       |
+-------------------------------+-------------------+-----------------------------+
|    111111000                  |    3              |     2                       |
+-------------------------------+-------------------+-----------------------------+
|    00111000                   |    3              |     0                       |
+-------------------------------+-------------------+-----------------------------+
|    11001110                   |    3              |     0                       |
+-------------------------------+-------------------+-----------------------------+
