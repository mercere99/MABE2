=================
What is a Module?
=================

MABE2 experiments are created by combining various types of modules. Modules contain the aspects of an experiment that 
cannot be held constant (i.e., requiring unique internal implementation). 

MABE2 implements six types of modules:
genomes, brains, evaluators, selectors, and placement modules. Modules of a given type are interchangeable, 
so switching from one brain type to another is as simple as changing a brain type parameter. This feature allows
users to focus their efforts specific aspects of their projects by only developing or modifying the modules of interest 
to them, by reusing existing modules when possible, and by not requiring detailed understanding of the entirety 
of MABE2. 

The implementation of modules is left entirely to users. As long as a user defined module conforms to the module 
interface, then there are no restrictions on how a module behaves. 

The typical experiment uses one module of each type, but this is not a requirement. 
Consider a foraging world where organisms divide whenever they collect enough resources. This world might 
manage reproduction locally and not rely on an optimizer. Or, consider a world where agents have a brain 
and world-defined sensors that are are placed using a genome. A user could configure MABE so that the 
agents had a single genome that was used to generate both the brain and sensor placement (allowing for 
genetic interactions), or the user could configure MABE so that the agent had two genomes (in which case 
the brain and sensor placement would be genetically independent).

The dynamics of evolution are defined by a combination of an `evaluation module <../evaluate/EvalNK.html>`_ , 
to determine how their phenotype will be assessed, and a
**link to selection page** to determine how that phenotype will
influence and organism's ability to move on to the next generation.


