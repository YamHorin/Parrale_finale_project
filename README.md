# project in parrale computing class 2023
</p>
<div align="center">
 <img alt="sequence problem
alignment" height="200px" src="https://upload.wikimedia.org/wikipedia/commons/thumb/7/79/RPLP0_90_ClustalW_aln.gif/1200px-RPLP0_90_ClustalW_aln.gif">
</div>
* this is a project that use MPI, openMP, cuda code
* this project is about comparing series (strings) of letters in the English alphabet.
is a simplification of algorithms used in bioinformatics where the goal is
Find similarities between biological molecules (proteins, DNA, RNA).
* In this context every letter
represents some chemical entity and a series of letters represents the structure of a molecule.
For example, a DNA molecule consists of four types of
nucleotides that are accepted to be marked with the letters A, T, C, G. and then the string
TCCGT can represent a segment of a DNA molecule.
* The problem of comparing series that we will deal with is known in biology as the sequence problem
alignment

## Various information files:
* data.txt-1000 different strings data2.txt-10 different strings
* Input.txt - the main information file - 4 strings of up to 3000 characters.
## how to run:
1. For your convenience, do Make all in order to create all the runtime files for testing
And if you want to run each version separately, do as follows:
2. Parallel version: will write make run - initialized to 4 output processes:
3. Sequential version: will write make normal_run - initialized to 1 output processes:
The above command to print has the following output:
It can be seen that the parallel version does
Faster than the serial version for large input
In both of the above programs, RANK=0 handles both input and output (for your convenience, the output is directed to txt files)
The inputs and outputs are arranged according to the same order of appearance and all the project conditions are met.
If you use make clean, it will clean all the running files for you at the end.
We created a struct called score_alignment and in addition we created MPI_Datatype:
which holds all the printed information.
Note also the parts of the code in which they are expressed:
