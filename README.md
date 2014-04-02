IF2DOT
=======

Generate a graph in DOT format (http://www.graphviz.org/doc/info/lang.html) that represents an IF description

#### Requirement ####
- Install IF (http://www-if.imag.fr) then set the environment variable IF to the install folder

#### Usage ####

The tool will generate DOT graphs for all IF processes in an IF description, one DOT graph  for one process

To generate all transition descriptions:

> if2dot fileName

To hide bodies of transitions ==> a smaller graph

> if2dot fileName bref
