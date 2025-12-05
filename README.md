## Project 5 - Empirical Analysis of Batch-then-Drain Profile

## Student Information
Parker Tennier  
ID: 008768156  
https://github.com/tennierp/hashtable-analysis.git

## Collaboration & Sources
Dr. Ali A. Kooshesh provided all of the benchmark tools necessary to run this program as well as the data structure implementations: HashTableDictionary, Operations, and the Main file. The project was built from his original HashMapAnalysis project. I also used a pdf-to-markdown converter to help me better organize my pdf report into a markdown file.

## Implementation Details
### Trace Generator
The trace generator creates an LRU trace file using a very large unique word files.
All traces are currently using seed 23 for reproduction, change this seed for different results.

### Harness
The harness was modified to accept a command-line argument specifying which profile to run.
The harness runs one untimed warmup run followed by the 7 timed trials.

## Testing & Status
This benchmark tests the HashTable single and double probing methods. It will properly generate 11 trace files.. After generating the trace files you can use the harness to cycle through those generated trace files and create a csv file at the end of it's runtime.

## Commands
Warning: This program needs the 20980712_uniq_words.txt words file or it will not properly run.

Here are the Makefile commands to run this program

Generate all trace files:
make generate-traces

Run the harness on all trace files:
make run-harness