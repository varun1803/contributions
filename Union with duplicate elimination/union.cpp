/*
	DOES NOT HANDLE DIFF
	LENGTH TUPLES USE MD5
	TO MAKE ALL SAME LEN

	M-WAY SORT USING BTREE
*/

#include <iostream>
#include <cstdio>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <unordered_set>
#include <set>
#include <map>
#include "btree.h"

#define pb push_back
#define mp make_pair

using namespace std;

int COLUMN_SIZE = 3; //ASSUMPTION
int BLOCK_SIZE = 1024*1024;
int RECORD_SIZE = 0;
int MM_SIZE;
int NO_OF_RECORDS_ALLOWED;

void split(string s, vector<string> &l, string delim = "\n")
{
	int i=0;
	char ch = delim[0];
	while(i < int(s.size()) ) 
	{
		string tmp = "";
		while(i < int(s.size()) && s[i] != ch)
		{
			tmp += s[i];
			i++;
		}
		i++;
		l.pb(tmp);
	}
}

int find_bucket(string val, int no_of_buckets, int level)
{
	int sum=0;
	for(int i = level; i<val.size(); i++)
	{
		sum += val[i];
	}
	return sum % no_of_buckets;
}

int find_no_of_records(string INPUT_FILE)
{
	ifstream file;
	int NO_OF_RECORDS = 0;
	file.open(INPUT_FILE.c_str());
	if (file.is_open())
	{
		const int SZ = BLOCK_SIZE;
		string buffer = "";
		buffer.resize(BLOCK_SIZE);
		file.read( &buffer[0], buffer.size() );
		while( int cc = file.gcount() ) {
			const char * p = &buffer[0];
			for ( int i = 0; i < cc; i++ ) {
				if ( p[i] == '\n' ) {
					NO_OF_RECORDS++;
				}
			}
			file.read( &buffer[0], buffer.size() );
		}
	}
	file.close();
	return NO_OF_RECORDS;
}

void hash_remove_dup(string table_name, int M, int level)
{
	ifstream t1_file;
	string buffer = "";
	buffer.resize(BLOCK_SIZE);
	vector <vector <string> > buckets(M-1);
	vector < shared_ptr<ofstream> > bucket_pointers;
	bucket_pointers.resize(M-1);

	//CHECK SIZE
	int no_of_rec = find_no_of_records(table_name);

	if (no_of_rec <= NO_OF_RECORDS_ALLOWED)
	{
		cout<<"REMOVING DUPLICATES IN "<<table_name<<endl;
		unordered_set<string> HASH;
		int bucket_no;
		t1_file.open(table_name.c_str());
		if (t1_file.is_open())
		{
			t1_file.read( &buffer[0], buffer.size() );
			while( int cc = t1_file.gcount() ) {
				vector<string> records;
				split(buffer, records);
				for (int i=0; i < (int)records.size(); i++)
				{
					if(isalnum(records[i][0]))
						HASH.insert(records[i]);
				}
				buffer="";
				buffer.resize(BLOCK_SIZE);
				t1_file.read( &buffer[0], buffer.size() );
			}
		}
		t1_file.close();

		//CREATE OUTPUT FILE
		string outfile_name = "union_" + table_name;
		ofstream outfile;
		outfile.open(outfile_name.c_str());
		for(auto it = HASH.begin();it!=HASH.end();it++) 
		{
			outfile<<*it<<"\n";
		}
		outfile.close();
		HASH.clear();
		return;
	}

	//PARTTION FILE
	for (int i=0; i<M-1; i++)
	{
		string fileName = table_name + "_" + to_string(i);
		bucket_pointers[i] = make_shared<ofstream>(fileName);
	}

	vector <string> SPECIAL(M-1, "");
	buffer.resize(BLOCK_SIZE);
	int bucket_no;
	t1_file.open(table_name.c_str());
	if (t1_file.is_open())
	{
		t1_file.read( &buffer[0], buffer.size() );
		while( int cc = t1_file.gcount() ) {
			vector<string> records;
			split(buffer, records);
			for (int i=0; i < (int)records.size(); i++)
			{
				bucket_no = find_bucket(records[i], M-1, level);
				if (isalnum(records[i][0]))
				{
					buckets[bucket_no].pb(records[i]);
					if (buckets[bucket_no].size() >= (BLOCK_SIZE/RECORD_SIZE) )
					{
						for (int j=0; j<buckets[bucket_no].size(); j++)
							*(bucket_pointers[bucket_no]) << buckets[bucket_no][j] << "\n";
						(buckets[bucket_no]).clear();
					}
				}
			}
			buffer="";
			buffer.resize(BLOCK_SIZE);
			t1_file.read( &buffer[0], buffer.size() );
		}
		t1_file.close();
	}

	for (int i=0; i<M-1; i++)
	{
		for (int j=0; j<buckets[i].size(); j++)
				*(bucket_pointers[i]) << buckets[i][j] << "\n";		
		(buckets[i]).clear();
		(*(bucket_pointers[i])).close();
	}


	if (level == RECORD_SIZE - 2)
	{
		cout<<"REMOVING DUPLICATES IN "<<table_name<<endl;
		ofstream outfile;
		outfile.open( ("union_" + table_name).c_str() );
		for(int i=0; i<M-1; i++)
		{
			
			buffer = "";
			unordered_set<string> HASH;
			buffer.resize(BLOCK_SIZE);
			t1_file.open( (table_name+"_"+to_string(i)).c_str() );
			if (t1_file.is_open())
			{
				t1_file.read( &buffer[0], buffer.size() );
				while( int cc = t1_file.gcount() ) {
					vector<string> records;
					split(buffer, records);
					for (int i=0; i < (int)records.size(); i++)
					{
						if(isalnum(records[i][0]))
							HASH.insert(records[i]);
					}
					buffer="";
					buffer.resize(BLOCK_SIZE);
					t1_file.read( &buffer[0], buffer.size() );
				}
			}
			t1_file.close();
			for(auto it = HASH.begin();it!=HASH.end();it++) 
				outfile<<*it<<"\n";
			HASH.clear();
			remove((table_name + "_" + to_string(i)).c_str());
		}
		outfile.close();
		return;
	}

	//RECURSIVELY REMOVE DUP FROM PARTTIONS
	for (int i=0; i<M-1; i++)
	{
		cout<<"PARTIONING "<<table_name<<" INTO : "<<table_name+"_"+to_string(i)<<endl;
		hash_remove_dup(table_name+"_"+to_string(i), M, level+1);
	}

	//TAKE UNION OF PARTTIONS
	ofstream outfile;
	int TEMP = 0;
	outfile.open( ("union_" + table_name).c_str() );
	for (int i=0; i<M-1; i++)
	{
		cout<<"COMBINING "<<table_name + "_" + to_string(i)<<" INTO "<<table_name<<endl;
		t1_file.open( ("union_" + table_name + "_" + to_string(i)).c_str() );
		if (t1_file.is_open())
		{
			t1_file.read( &buffer[0], buffer.size() );
			while( int cc = t1_file.gcount() ) {
				vector<string> records;
				split(buffer, records);
				for (int j=0; j < (int)records.size(); j++)
				{
					if(isalnum(records[j][0]))
					{
						outfile << records[j] << "\n";
						TEMP++;
					}
				}
				buffer="";
				buffer.resize(BLOCK_SIZE);
				t1_file.read( &buffer[0], buffer.size() );
			}
			t1_file.close();
		}
		remove(("union_" + table_name + "_" + to_string(i)).c_str());
		remove((table_name + "_" + to_string(i)).c_str());
	}
	outfile.close();
}

string merge_parts(vector <string> file_names, int start, int end)
{
	if (end-start+1 <= NO_OF_RECORDS_ALLOWED )
	{
		cout<<"IT FITS"<<endl;
		set<string> HASH;
		map<string, int> FILE_MAP;
		
		vector <ifstream> fps(end-start+1);
		int total = 0;
		for(int i=start; i<=end; i++)
		{
			fps[i-start].open(file_names[i]);
			total += find_no_of_records(file_names[i]);
		}
		cout<<"TOTAL : "<<total<<endl;
		cout<<"NO OF FILES : "<<fps.size()<<endl;
		ofstream outfile;
		string fname = "merge_" + to_string(start) + "_" + to_string(end);
		outfile.open(fname);
		string b;
		int NN=0, NNN=0;
		for(int i=0; i<end-start+1; i++)
		{
			cout<<"INSERT LINE"<<endl;
			//fps[i].getline(b, 5000, '\n');
			if (fps[i].peek() != EOF)
			{
				getline(fps[i], b);
				NNN++;
				while( HASH.find(string(b)) != HASH.end() )
				{
					//fps[i].getline(b, 5000, '\n');
					if (fps[i].peek() != EOF)
					{
						getline(fps[i], b);
						NNN++;
					}
					else
						break;
				}
				if( HASH.find(string(b)) == HASH.end() )
				{
					HASH.insert(string(b));
					FILE_MAP[string(b)] = i;
				}
			}
		}
		cout<<HASH.size()<<" "<<NNN<<endl;

		while (true)
		{
			while(!HASH.empty())
			{
				string s = *(HASH.begin());
				int i = FILE_MAP[s];
				
				outfile << s << endl;
				NN++;

				//fps[i].getline(b, 5000, '\n');
				if (fps[i].peek() != EOF)
				{
					getline(fps[i], b);
					NNN++;
					while( HASH.find(string(b)) != HASH.end() )
					{
						//fps[i].getline(b, 5000, '\n');
						if (fps[i].peek() != EOF)
						{
							getline(fps[i], b);
							NNN++;
						}
						else
							break;
					}
					if( HASH.find(string(b)) == HASH.end() )
					{
						HASH.insert(string(b));
						FILE_MAP[string(b)] = i;
					}
				}

				HASH.erase(s);
				FILE_MAP.erase(s);
			}
			bool flag = true;
			for (int i=0; i<end-start+1; i++)
			{
				if (fps[i].peek() != EOF)
				{
					flag = false;
					//fps[i].getline(b, 5000, '\n');
					getline(fps[i], b);
					NNN++;
					HASH.insert(string(b));
					FILE_MAP[string(b)] = i;
				}
			}
			if (flag)
				break;
		}

		outfile.close();

		for(int i=start; i<=end; i++)
			remove(file_names[i].c_str());

		cout<<NNN<<" "<<NN<<endl;
		return fname;
	}

	vector <string> smaller_parts;
	int M = NO_OF_RECORDS_ALLOWED;
	for(int i=0; i<file_names.size(); i+=M)
	{
		smaller_parts.pb(merge_parts(file_names, i, min(int(file_names.size())-1, i+M-1) ));
	}
	return merge_parts(smaller_parts, 0, smaller_parts.size()-1);
}

void sort_remove_dup(string table_name)
{
	ifstream t1_file;
	string buffer = "";
	buffer.resize(BLOCK_SIZE);

	int part_no = 0;
	BTree *t = new BTree(5);
	vector <string> file_names;

	//SORT PARTS
	int NNN=0, NN=0;
	t1_file.open( table_name.c_str() );
	if (t1_file.is_open())
	{
		t1_file.read( &buffer[0], buffer.size() );
		while( int cc = t1_file.gcount() ) {
			vector<string> records;
			split(buffer, records);
			for (int j=0; j < (int)records.size(); j++)
			{
				if(isalnum(records[j][0]))
				{
					(*t).insert(records[j]);
					NNN++;
				}
				if ( (*t).size == NO_OF_RECORDS_ALLOWED )
				{
					cout<<(*t).size<<endl;
					ofstream f;
					string fname = table_name + "_" + to_string(part_no);
					part_no++;
					file_names.pb(fname);
					f.open((fname).c_str());
					queue <string> q;
					(*t).traverse(q);
					while (!q.empty())
					{
						f<<q.front()<<"\n";
						q.pop();
						NN++;
					}
					f.close();
					delete(t);
					t = new BTree(5);
					cout<<(*t).size<<endl;
				}
			}
			buffer="";
			buffer.resize(BLOCK_SIZE);
			t1_file.read( &buffer[0], buffer.size() );
		}
		t1_file.close();

		if ( (*t).size != 0 )
		{
			cout<<(*t).size<<endl;
			ofstream f;
			string fname = "temp_" + table_name + "_" + to_string(part_no);
			part_no++;
			file_names.pb(fname);
			f.open((fname).c_str());
			queue <string> q;
			(*t).traverse(q);
			while (!q.empty())
			{
				f<<q.front()<<"\n";
				q.pop();
				NN++;
			}
			f.close();
			delete(t);
			t = new BTree(5);
			cout<<(*t).size<<endl;
		}
	}
	cout<<NNN<<" "<<NN<<endl;

	//JOIN PARTS
	string outfile_name = merge_parts(file_names, 0, file_names.size()-1);
	system( ("mv " + outfile_name + " union_" + table_name).c_str() );
}

void union_table(string t1, string t2, int n, int M, string type)
{
	if (RECORD_SIZE == 0) RECORD_SIZE = (COLUMN_SIZE+1)*n;
	NO_OF_RECORDS_ALLOWED = (BLOCK_SIZE * (M-1))/RECORD_SIZE;

	//CONCATENTATE BOTH FILES
	string temp_file = t1 + "_" + t2;
	string command = "cat " + t1 + " " + t2 + " >> " + temp_file;
	system(command.c_str());

	if (type == "hash")
	{
		//REMOVE_DUP
		hash_remove_dup(temp_file, M, 0);
		remove(temp_file.c_str());
	}
	else
	{
		cout<<"calling b-tree function\n";
		sort_remove_dup(temp_file);
		remove(temp_file.c_str());
	}
}

int main(int argc, char *argv[])
{
	union_table(argv[1], argv[2], stoi(argv[3]), stoi(argv[4]), argv[5]);
	return 0;
}