# Evaluation Modules

Evaluation modules are categorized by the types of IO they work with.

## Static modules (static/)

All of the static modules do not take ANY input and only produce output for
solving a single problem that does not change (typically of most evolutionary
computation systems).  Currently there are two types of static output that we
use of types `BitVector` and `emp::vector<double>`.

## Value IO (value_io/)

These evaluation modules provide a set of doubles (`emp::vector<double>` or
`std::unordered_set<size_t,double>`) as input, and expect a new vector of
doubles back as an output.

In some cases, these are used for choosing a move in a game, where the output
vector lists all possible moves and the highest value of a legal move is the
one that is automatically chosen.  Such as system allows the organism to simply
weight each possible move without needing to actually select one.