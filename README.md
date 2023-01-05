# code-capsule-reconfiguration-of-SFCs

For performance purposes, it was chosen to develop a program in C++ in a structured and non-object-oriented way.
The program has been tested on Ubuntu 20.4. It cannot be said if there are dependencies to be installed in other OS versions.
note that it is necessary to have g++ installed, as well as the solver CPLEX 12.6 (it has not been tested with other versions). 
Also note that you may need to update the CPLEX paths.

This program consists of 4 files: io.h, ilp.h, aux.h and main.cpp

To compile use: "g++ -O -fPIC -fexceptions -DNDEBUG -DIL_STD -I/opt/ibm/ILOG/CPLEX_Studio126/cplex/include -I/opt/ibm/ILOG/CPLEX_Studio126/concert/include *.cpp -o saida -L/opt/ibm/ILOG/CPLEX_Studio126/cplex/lib/x86-64_linux/static_pic -lilocplex -lcplex -L/opt/ibm/ILOG/CPLEX_Studio126/concert/lib/x86-64_linux/static_pic -lconcert -lcplex -lm -lpthread -g -w"

To run do: ./saida 3 10000 
In this case "saida" is the compiled object, "3" is the type of reconfiguration applied, and "10000" is the size of the time window to be considered in the analysis.

The reconfiguration type is passed as an argument. 1, to only reconfigure the size of the VNF instances, 2 to reconfigure the instance sizes and routings, and 3 to reconfigure the entire model. As already described in the article, mode 3 is very computationally expensive and can run for a long time.

Depending on the machine used, this time can be very long. To mitigate this problem in the "ilp.h" file, multithreading can be used, indicating on line 406 how many threads to use. Additionally, you can stipulate a maximum execution time per solver call, uncommenting line 414 and inserting the tolerated execution time in seconds.

The file paths are passed statically, inside the main (in the code), and can be changed, if necessary.

The other parameters and values are all static, and can be changed within the code.
