Author: Dillon Welch
E-mail: daw027@latech.edu

Everything should be working properly. This is the final round of improvements on code that I started working on in CSC 222. Dr. Duncan gave us a working tokenizer. James Poore, Nathan Lapp, along with myself, worked on the various aspects that we had to do for our final project as a group (variable substitution, piping, builtins, etc). After getting feedback and debugging help from Dr. Duncan after we turned it in, I further improved the code (involving swapping in a few functions from ProgramTechShell from your website so my code would be cleaner) as well as added PWD and redirects.

In the Input and Output directories, there are input and output files you can use to test my shell.
Shell.1 through Shell.6 are files that Dr. Duncan provided last quarter for testing. My corresponding output is shell1.out through shell6.out (and shell6.err).
redirect.in is a file I made to show that redirection works. This creates three files: fileA, f00, and fooTest. You can have these printed in the shell by running the catTest.in file, and catTest.out is the corresponding output file.

There is a make file located in the main directory. The folder "ProgramTechShell" is starter code.
