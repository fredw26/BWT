#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <string.h>
using namespace std;

#define B_SIZE 250000000
#define INDEX_FREQ 7
unsigned int genSize;
char* genome;

bool suffCompare(const char* a, const char* b) {
    unsigned int i = 0;

    while(a[i] == b[i] && a[i] != '$')
    {
        i++;
    }
    return(a[i] <= b[i]);
}

int main(int argc, char* argv[])
{
    if(argc != 4)
    {
        cerr << "Must provide path, number of files, and number of lines per file";
        return 1;
    }

    const string filePath(argv[1]);
    const int numFiles = atoi(argv[2]);
    const int numLines = atoi(argv[3]);

    unsigned int genIter = 0;
    for(int i = 1; i <= numFiles; i++)
    {
        string line;
        string file;
        if(i < 10)
            file = filePath + string(1, (char)(i + '0')) + string(".txt");
        else
            file = filePath + string(1, (char)((i/10) + '0')) + string(1, (char)((i%10) + '0')) + string(".txt");

        cerr << "READING FILE " << file << endl;
        ifstream infile(file.c_str());

        if(!getline(infile, line) || !getline(infile, line))
        {
            cerr << "File error\n";
            return 1;
        }

        int linesRead = 0;
        if(i == 1)
        {
            getline(infile, line);
            genSize = (line.length()*numLines*numFiles) + 1;
            genome = new char[genSize];
            for(int k = 0; k < line.length(); k++)
            {
                genome[genIter] = line[k];
                genIter++;
            } 
            linesRead++;
        }

        while(getline(infile, line) && linesRead < numLines)
        {
            for(int k = 0; k < line.length(); k++)
            {
                genome[genIter] = line[k];
                genIter++;
            }
            linesRead++;
        }
    }

    genome[genIter] = '$';

    cerr << "CREATING C ARRAY" << endl;

    const int rSize = (genSize/B_SIZE)*2;

    char** C = new char* [rSize];
    for(int i = 0; i < rSize; i++)
    {
        C[i] = genome+(i*(genSize/rSize));
    }

    cerr << "SORTING C ARRAY" << endl;

    sort(C, C+rSize, suffCompare);        

    cerr << "CREATING B ARRAY" << endl;

    char** B = new char* [B_SIZE];

    unsigned int count = 0;

    for(int i = 0; i <= rSize; i++)
    {
        unsigned int bIter = 0;

        cerr << "SELECTING BIN " << i << " of " << rSize << endl;
        cerr << "DIVIDER " << (C[i] - genome) << endl;

        for(unsigned int j = 0; j < genSize; j++)
        {
            if((i == rSize || suffCompare(genome+j, C[i])) && C[i] != genome+j &&
               (i == 0 || (C[i-1] != genome+j && suffCompare(C[i-1], genome+j))))
            {
                if(bIter < B_SIZE-1 || (i == rSize && bIter < B_SIZE))
                {
                    B[bIter] = genome+j;
                    bIter++;
                }
                else
                {
                    cerr << "TOO MANY INDICES, FINDING MEDIAN\n";
                    sort(B, B+bIter, suffCompare);
                    bIter /= 2;
                    if(i == rSize)
                    {
                        i--;
                        C[i-1] = C[i];
                    }
                    C[i] = B[bIter];
                    cerr << "DIVIDER IS NOW " << (C[i] - genome) << endl;
                    j--;
                }
            }
        }

        if(i != rSize)
        {
            B[bIter++] = C[i];
        }

        cerr << "SORTING BIN " << i << ", " << bIter << " indices\n";

        sort(B, B+bIter, suffCompare);

        cerr << "PRINTING BIN " << i << endl;

        for(int y = 0; y < bIter; y++)
        {
            if(B[y] == genome)
            {
                cout << '$';
            }
            else
            {
                cout << *(B[y] - 1);
            }
        }
        
        count += bIter;
        cerr << genSize - count << " indices remaining\n";
    }
}
