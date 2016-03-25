#include<cstdio>
#include<memory>
#include<cstring>
#include<string>
#include<iostream>
#include<sstream>
#include<fstream>
#include<cstdlib>
#include<algorithm>
#include<vector>
#include<queue>
#include<unordered_map>
#include<map>
#include<climits>

using namespace std;
unordered_map<string, pair<int, string> > schema;
unordered_map<string, char*> HashMapArgs;
vector<string> sortByColumns;
vector<int> ColsIndex;
int recordSize=0, RAM, numRecords, NumBlocks,order=0;
vector<ifstream* > filePointers;



vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}
int LengthInBytes(string str) {
    if (str == "date")
        return 10;
    else if (str == "int") 
        return 4;
    else 
    {
        int flag = 0;
        string num="";
        int x = str.find("(");
        int y = str.find(")");
        for(int i=x+1;i<y;i++)
            num+=str[i];    
        return atoi(num.c_str());
        
    }
    
}
void readMetaData() 
{
    int index = 0;
    
    FILE *fptr;
    string filename = HashMapArgs["meta_file"],name="",type="";
    fptr = fopen(filename.c_str(), "r");


    char ch ;
    int icolSize;
    int flag = 0;
    bool colNameFlag = true;

    while((ch = fgetc(fptr)) != EOF)
    {
        if (ch == ',') 
            flag = 1;

        else if (ch == '\n')
        {
            int val  = LengthInBytes(type);
            recordSize =val +recordSize ;
            pair<int,string> mypair;
            mypair.first  = index;
            mypair.second = type;
            schema[name] = mypair;
            type=name = "";
            ++index;
            flag = 0;
        }
        else
        {
            if(ch != ' ')
            {
                if(flag == 0)
                    name +=ch;
                else
                    type += ch;
            }            
        }        
    }
    fclose(fptr);
    string str = HashMapArgs["output_column"];
    vector<string> OutputColumns = split(str,',');
    int val,i=0 ;
    while(i<OutputColumns.size())
    {
        val = schema[OutputColumns[i]].first;
        ColsIndex.push_back(val);
        ++i;
    }
    
}
bool Compare(const vector<string>& rec1, const vector<string>& rec2, int columnIndex, bool forHeap)
{
    //Base Case;

    if (columnIndex >= sortByColumns.size()) 
        return false;
    
    bool ret;
    string str = sortByColumns[columnIndex];

    int index1 = schema[str].first;
    int index2 = schema[str].first;

    string str1 = rec1[index1];
    string str2 = rec2[index2];
    string type = schema[str].second;

    if (str1 == str2)
    {
        ret = Compare(rec1, rec2, columnIndex + 1, forHeap);
        return ret;
    }
    

    if (type == "date")
    {
        
        vector<string> date1 = split(str1, '/');
        vector<string> date2 = split(str2, '/');
        int val = date1.size()-1;
        for(int i=val; i>=0; i--)
        {
            if (date1[i] == date2[i])
            {
                ;
            } 
            else 
            {
                if (order == 1) 
                    ret = date1[i] < date2[i] ;
                else 
                    ret = date1[i] > date2[i];
                
                break;
            }
        }

        if(forHeap)
            return (ret+1)%2;
        else
            return ret;
    }
    else
    {
        
        if (order)
        {
            ret = (str1 < str2); 
              if(forHeap)
                return (ret+1)%2;
              else
                return ret;
        } 
        else 
        {
            ret = str1 > str2;
            if(forHeap)
                return (ret+1)%2;
            else
                return ret;
        }
    }
}

bool compareRecords(const vector<string>& r1, const vector<string>& r2)
{ 
    return Compare(r1, r2, 0, false);
}

bool heapCompare(const pair<vector<string>, int>& p1, const pair<vector<string>, int>& p2) 
{
    return Compare(p1.first, p2.first, 0, true);
}
priority_queue<pair<vector<string>, int>, vector<pair<vector<string>, int> >, decltype(&heapCompare)> maxheap(&heapCompare);


void mergeFiles()
{
    int sizeOutBuffer = 0.2 * RAM;
    
    int numOB = sizeOutBuffer/recordSize;
    
    
    double x = ((double)((RAM-sizeOutBuffer)/NumBlocks));
    int numFilePtr = x/recordSize;
  
    // Store all file pointers in an array
    ifstream *file ;
    string str;
    for(int i=0; i<recordSize; i++) 
    {
        str = to_string(i+1);
        file = new ifstream(("split/file" + str).c_str());
        filePointers.push_back(file);
    }

    ifstream *fptr;
    string line;
    vector<string> record;
    int counter = 0;
    int countOutBuff = 0;
    
    for(int i=0; i<NumBlocks; i++)
    {
        fptr = filePointers[i];
        counter = 0;      
        
        while(getline(*fptr, line)) 
        {
            
            record = split(line, ',');
            maxheap.push(make_pair(record, i));
            ++counter;
            if (counter  == numFilePtr)
                break;
            
        }
    }
    

    
    ofstream outFile;
    outFile.open(HashMapArgs["output_file"]);
    string tempR;
    pair<vector<string>, int> mypair;
    vector<vector<string> > Records;

   
    while(!maxheap.empty()) 
    {
        mypair = maxheap.top();
        maxheap.pop();
        Records.push_back(mypair.first);
        
        // add new record from filepointer to heap
        if (getline(*filePointers[mypair.second], line))
        {
            record = split(line, ',');
            maxheap.push(make_pair(record, mypair.second));
        } 

        countOutBuff=1+countOutBuff;

        if (countOutBuff == numOB) 
        {
            
            for(int i=0; i<Records.size(); i++)
            {
                // only put specific columns into the file
                str = Records[i][ColsIndex[0]];

                for(int j=1; j<ColsIndex.size(); j++)
                    str += "," + Records[i][ColsIndex[j]];
                
                str = str + "\n";
                outFile << str;
            }

            
        }   
    }

    
    if (countOutBuff < numOB)
    {
        
        for(int i=0; i<Records.size(); i++)
        {
            str = Records[i][ColsIndex[0]];
            
            for(int j=1; j<ColsIndex.size(); j++) 
                str += "," + Records[i][ColsIndex[j]];
            
            str += "\n";
            outFile << str;
        }
        Records.clear();
        countOutBuff = 0;
     
    }

    int i=0;
    while(i<NumBlocks) 
        delete filePointers[i++];    
    
    outFile.close();
    filePointers.clear();
}

void splitIntoBlocks()
{
    int counter;
    string line, RecordTemp ;

    string inputFile = string(HashMapArgs["input_file"]);
    ofstream out;
    ifstream f(inputFile.c_str());
    vector<vector<string> > records;
    vector<string> record;
    counter = 0;

    while(getline(f, line))
    {
        
        record = split(line, ',');
        records.push_back(record);
        counter++;

        if (numRecords == counter )
        {
            counter = 0;
            sort(records.begin(), records.end(), compareRecords);
           
            out.open(("split/file/"  + to_string(NumBlocks)).c_str());
            RecordTemp ="";
            int len,val=records.size();
            for(int i=0; i<val; i++)
            {
                RecordTemp = records[i][0];
                len = records[i].size();

                for(int j=1; j<len; j++) 
                    RecordTemp = RecordTemp + "," + records[i][j];
0
                string str = RecordTemp +"\n";
                RecordTemp = str;
                out << RecordTemp;
            }
            out.close();
            records.clear();
            NumBlocks++;
        }
    }
    if (counter != 0)
    {
        counter = 0;
        ++NumBlocks;

        out.open(("split/file" + to_string(NumBlocks)).c_str());
        int len = records.size(),val;

        for(int i=0; i < len; i++)
        {
            RecordTemp = records[i][0];
            val = records[i].size();

            for(int j=1; j< val ; j++) 
                RecordTemp =RecordTemp+"," + records[i][j];
            
            string str = RecordTemp +"\n";
            RecordTemp = str;
            out << RecordTemp;
        }
        out.close();
        sort(records.begin(), records.end(), compareRecords);
        records.clear();
    }
}






int main(int numArgs, char *args[]) {

    ios_base::sync_with_stdio(0);
    
    
    // Arguments parse

    
    string str="";
    int len;
    
        for(int i=1; i<numArgs; i++)
        {
            
            if (args[i][0] == '-' && args[i][1]=='-')
            {
                               
                str = "";
                len = strlen(args[i]);
                for(int j=2; j<len; j++)
                    str += args[i][j];                            
            }
            else 
                HashMapArgs[str] = args[i];       
        }

        str = HashMapArgs["sort_column"];
        sortByColumns = split(str, ',');
        str = HashMapArgs["order"];
       
        if(str == "asc")
          order = 1;
        else 
          order = 0;


    // Read From metafile

    readMetaData();  

    double memory = atoi(HashMapArgs["mm"]);
    RAM = ((memory * 1024 * 1024) * 0.8);
    numRecords = RAM/recordSize;
    
    splitIntoBlocks();

  //  mergeFiles();
    return 0;
}
