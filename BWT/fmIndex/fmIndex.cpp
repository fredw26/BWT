#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <iostream>
using namespace std;

#define CHECKPOINT_FREQ 448
#define INDEX_FREQ 7

struct checkpoint {
    unsigned int a;
    unsigned int c;
    unsigned int g;
    unsigned int t;
};

unsigned int totalA;
unsigned int totalC;
unsigned int totalG;
unsigned int totalT;
unsigned int startPos;
unsigned int genomeSize;

char* genome;
checkpoint* cpArray;
unsigned int* positionArray;

unsigned int walkLeft(unsigned int bwtIndex);

int initializeFmIndex(string filename) {

    ifstream infile(filename.c_str());
    cout << "LOADING GENOME\n";
    
    infile.seekg(0, infile.end);
    genomeSize = (unsigned int)infile.tellg();
    cout << "GENOME SIZE: " << genomeSize << endl;
    infile.seekg(0, infile.beg);
    genome = new char[genomeSize];
    infile.read(genome, genomeSize);   
    infile.close();
    
    cpArray = new checkpoint[(genomeSize/CHECKPOINT_FREQ)+1];
    positionArray = new unsigned int[(genomeSize/INDEX_FREQ)+1];

    cout << "FILE LOADED\n";
    
    //LOAD TRANSFORM INTO DATA STRUCTURE
    totalA = 0;
    totalC = 0;
    totalG = 0;
    totalT = 0;

    unsigned int a = 0;
    unsigned int c = 0;
    unsigned int g = 0;
    unsigned int t = 0;

    for(unsigned int i = 0; i < genomeSize; i++)
    {
        switch (genome[i]) {
            case 'A':
                totalA++;
                a++;
                break;
            case 'C':
                totalC++;
                c++;
                break;
            case 'G':
                totalG++;
                g++;
                break;
            case 'T':
                totalT++;
                t++;
                break;
            case '$':
                startPos = i;
                break;
            default:
                printf("Invalid base character 2\n");
                exit(1);
        }

        if(i % CHECKPOINT_FREQ == 0)
        {
            cpArray[i/CHECKPOINT_FREQ].a = a;
            cpArray[i/CHECKPOINT_FREQ].c = c;
            cpArray[i/CHECKPOINT_FREQ].g = g;
            cpArray[i/CHECKPOINT_FREQ].t = t;
        }
    }

    cout << "GENOME LOADED AND CHECKPOINTS CREATED\n";
    //IF INDEX FILE EXISTS, PARSE IT OTHERWISE:

    FILE * inputFile = fopen("index.txt", "rb");
    if(inputFile)
    {
        cout << "READING INDEX FILE\n";
        if(fread(positionArray, 4, (genomeSize/INDEX_FREQ)+1, inputFile) != (genomeSize/INDEX_FREQ)+1)
        {
          cout << "ERROR LOADING INDEX FILE\n";
          exit(1);
        }
        fclose(inputFile);
    }
    else
    {
      cout << "CREATING INDEX FILE\n";

      unsigned int curIndex = 0;
      unsigned int count = genomeSize-1;
      while(count > 0)
      {
          if(curIndex % INDEX_FREQ == 0)
          {
              positionArray[curIndex/INDEX_FREQ] = count;
          }
          curIndex = walkLeft(curIndex);
          count--;
          if(count % (genomeSize/50) == 0)
            cout << count << " bases left to process, " << count / (genomeSize/50) << "/50 remaining\n";
      }

      if(curIndex % INDEX_FREQ == 0)
      {
          positionArray[curIndex/INDEX_FREQ] = count;
      }
 
      inputFile = fopen("index.txt", "wb");
      fwrite(positionArray, 4, (genomeSize/INDEX_FREQ)+1, inputFile);
      fclose(inputFile);
    }

    cout << "INDEX COMPLETED\n";

    return 0;
}

unsigned int getPosition(unsigned int bwtIndex) {
    if(genome[bwtIndex] == '$')
        return 0;
    unsigned int curIndex = bwtIndex;
    unsigned int offset = 0;
    while(curIndex % INDEX_FREQ > 0)
    {
        curIndex = walkLeft(curIndex);
        offset++;
    }

    return positionArray[curIndex/INDEX_FREQ] + offset;
}

unsigned int getRank(unsigned int bwtIndex) {
    unsigned int count = 0;
    char base = genome[bwtIndex];
    for(unsigned int offset = 1; offset <= bwtIndex % CHECKPOINT_FREQ; offset++)
    {
        if(base == genome[bwtIndex-offset+1])
            count++;
    }

    switch (base) {
        case 'A':
            return cpArray[bwtIndex/CHECKPOINT_FREQ].a + count - 1;
        case 'C':
            return cpArray[bwtIndex/CHECKPOINT_FREQ].c + count - 1;
        case 'G':
            return cpArray[bwtIndex/CHECKPOINT_FREQ].g + count - 1;
        case 'T':
            return cpArray[bwtIndex/CHECKPOINT_FREQ].t + count - 1;
        case '$':
            return 0;
        default:
            printf("Invalid base character 3\n");
            exit(1);
    }
}

unsigned int getRanked(unsigned int rank, char base) {
    rank++;
    unsigned int i;
    unsigned int curRank;
    unsigned int prevRank;
    for(i = 0; i < (genomeSize/CHECKPOINT_FREQ)+1; i++)
    {
        switch (base) {
            case 'A':
                curRank = cpArray[i].a;
                break;
            case 'C':
                curRank = cpArray[i].c;
                break;
            case 'G':
                curRank = cpArray[i].g;
                break;
            case 'T':
                curRank = cpArray[i].t;
                break;
            case '$':
                return startPos;
            default:
                printf("Invalid base character 3\n");
                exit(1);
        }

        if(curRank == rank && genome[i*CHECKPOINT_FREQ] == base)
        {
            return i*CHECKPOINT_FREQ;
        }
        else if(curRank >= rank)
        {
            unsigned int bwtIndex = (i-1)*CHECKPOINT_FREQ;
            curRank = prevRank;
            while(rank > curRank && bwtIndex < genomeSize)
            {
                bwtIndex++;
                if(base == genome[bwtIndex])
                    curRank++;
            }
            return bwtIndex;
        }
        prevRank = curRank;
    }

    unsigned int bwtIndex = (i-1)*CHECKPOINT_FREQ;
    curRank = prevRank;
    while(rank > curRank && bwtIndex < genomeSize)
    {
        bwtIndex++;
        if(base == genome[bwtIndex])
            curRank++;
    }
    return bwtIndex;
    
}

unsigned int walkLeft(unsigned int bwtIndex) {
    switch (genome[bwtIndex]) {
        case 'A':
            return getRank(bwtIndex) + 1;
        case 'C':
            return getRank(bwtIndex) + totalA + 1;
        case 'G':
            return getRank(bwtIndex) + totalA + totalC + 1;
        case 'T':
            return getRank(bwtIndex) + totalA + totalC + totalG + 1;
        case '$':
            return 0;
        default:
            printf("Invalid base character 4\n");
            exit(1);
    }
}

unsigned int walkRight(unsigned int bwtIndex) {
    char base;
    unsigned int rank;
    if(bwtIndex == 0)
        return startPos;
    else if(bwtIndex < totalA + 1)
    {
        base = 'A';
        rank = bwtIndex - 1;
    }
    else if(bwtIndex < totalA + totalC + 1)
    {
        base = 'C';
        rank = bwtIndex - totalA - 1;
    }
    else if(bwtIndex < totalA + totalC + totalG + 1)
    {
        base = 'G';
        rank = bwtIndex - totalA - totalC - 1;
    }
    else
    {
        base = 'T';
        rank = bwtIndex - totalA - totalC - totalG - 1;
    }
    
    return getRanked(rank, base);
}

unsigned int alignRead(string read, int errorsAllowed) {
    if(errorsAllowed < 0 || errorsAllowed >= read.length() || read.length() == 0)
    {
        printf("Invalid input\n");
        exit(1);
    }

    for(int z = errorsAllowed; z >= 0; z--)
    {
        int stringStart = z * (read.length() / (errorsAllowed+1));
        int stringEnd;
        if(z == errorsAllowed)
            stringEnd = read.length() - 1;
        else 
            stringEnd = stringStart + (read.length() / (errorsAllowed+1)) - 1;

        unsigned int startIndex = 0;
        unsigned int endIndex = genomeSize - 1;

        while(startIndex < genomeSize-1 && genome[startIndex] != read[stringEnd])
            startIndex++;

        while(endIndex > 0 && genome[endIndex] != read[stringEnd])
            endIndex--;

        int stringIter = stringEnd;
        while(stringIter > stringStart && startIndex <= endIndex)
        {
            switch(read[stringIter]) {
                case 'A':
                    startIndex = getRank(startIndex) + 1;
                    endIndex = getRank(endIndex) + 1;
                    break;
                case 'C':
                    startIndex = getRank(startIndex) + totalA + 1;
                    endIndex = getRank(endIndex) + totalA + 1;
                    break;
                case 'G':
                    startIndex = getRank(startIndex) + totalA + totalC + 1;
                    endIndex = getRank(endIndex) + totalA + totalC + 1;
                    break;
                case 'T':
                    startIndex = getRank(startIndex) + totalA + totalC + totalG + 1;
                    endIndex = getRank(endIndex) + totalA + totalC + totalG + 1;
                    break;
                default:
                    printf("Invalid base character: %c\n", read[stringIter]);
                    exit(1);
            }

            stringIter--;

            while(startIndex < genomeSize-1 && genome[startIndex] != read[stringIter])
                startIndex++;
    
            
            while(endIndex > 0 && genome[endIndex] != read[stringIter])
                endIndex--;
        }


        while(startIndex <= endIndex)
        {
            if(stringStart < getPosition(startIndex) && 
               genomeSize >= getPosition(startIndex) + (stringEnd-stringStart+1) &&
               read.length() - stringEnd - 1 <= genomeSize - (getPosition(startIndex) + (stringEnd-stringStart+1)))
            {
                unsigned int startIter = startIndex;
                int errors = 0;

                if(stringStart > 0)
                {
                    //LOOK LEFT
                    startIter = walkLeft(startIter);
                    stringIter = stringStart-1;
                    while(stringIter >= 0 && startIter > 0)
                    {
                        if(genome[startIter] != read[stringIter])
                        {
                            errors++;
                            if(errors > errorsAllowed)
                                break;
                        } 
                        startIter = walkLeft(startIter);
                        stringIter--;
                    }
                    startIter = walkRight(startIter);
                }
                
                if(stringEnd < read.length() - 1 && errors <= errorsAllowed)
                {
                    //LOOK RIGHT 
                    unsigned int endIter = startIndex;
                    stringIter = stringEnd + 1;
                    for(int i = 0; i < stringEnd - stringStart + 1; i++)
                        endIter = walkRight(endIter);

                    while(stringIter < read.length() && endIter < genomeSize)
                    {
                        if(read[stringIter] != genome[endIter])
                        {
                            errors++;
                            if(errors > errorsAllowed)
                                break;
                        }
                        endIter = walkRight(endIter);
                        stringIter++;
                    }
                }

                if(errors <= errorsAllowed)
                {
                    fprintf(stderr, "String '%s' found at position %u\n", read.c_str(), getPosition(startIter)-1);
                    return getPosition(startIter)-1;
                } 
            }

            do {
                startIndex++;
            } while(startIndex <= endIndex && genome[startIndex] != read[stringStart]);
        }
    }
    return -1;
}

int main(int argc, char* argv[])
{
    initializeFmIndex(argv[1]);
    string read;
    int errors;
    ifstream infile(argv[2]);
    unsigned int found = 0;
    unsigned int total = 0;
    while(getline(infile, read))
    {
        if(alignRead(read, 2) != -1)
          found++;

        total++;
        if(total % 50000 == 0)
          cout << "READ " << total << endl;
    }
        
    cout << "FOUND " << found << " OUT OF " << total << endl;
}
