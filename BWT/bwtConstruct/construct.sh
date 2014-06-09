rm -rf bwt
g++ -O3 bwtConstruct.cpp -o bwt
time ./bwt ~/CS/124/project/genome/ref_genomeH 46 825000 > transform
