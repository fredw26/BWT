rm output
g++ -O3 -o index fmIndex.cpp
time ./index ~/CS/124/project/chr1-bwt ~/CS/124/project/chr1-reads 2> output
