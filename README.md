# Parallel Histogram Computation

This program is an implementation of parallel computation using processes. It takes a list of file names as input and produces a new set of files with the naming
convention filePID.hist containing the English alphabet histograms for each input file, where PID is the process ID of the corresponding child process. This code was
developed for an academic assignment for the CIS*3110 or Operating Systems course.

## Compilation

To compile the program, use the provided Makefile with the following command: `make`

## Usage

The program accepts file names as command-line arguments. For example:
`./A1 file1.txt file2.txt file3.txt`

If no input files are provided, the program will display an error message and exit.

The program uses multiple processes - a parent and multiple children - for parallel computation. The parent process forks a child for each file, which computes the
histogram and sends it back to the parent through pipes. The parent saves the histogram data to output files.

Special inputs can be used to send signals to specific children. For example, the string "SIG" followed by a file name will send a SIGINT signal to the corresponding
child, causing it to terminate.
