#include <iostream>
#include <array>
#include <iomanip>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>

using namespace std;

/* C++11 required!
   input file format: id,cpc,level (int,string,int)
   cpc: Xsssc[*]/gg[*] 
        where X = section, sss = scheme, c[*] = class, gg[*] = group
*/

struct data {
  string id;
  string cpc1;
  string section;
  string scheme;
  string clas;
  string group;
  string level;
  int lvl;
  int iblock;
  array<unsigned char,13> path;
};

data* parse(string s);

int main(int argc, char* argv[])
{
  array<unsigned char,13> zeropath;
  array<unsigned char,13> path;
  zeropath.fill('0');

  ifstream infile(argv[1]);
  ofstream outfile(argv[2]);

  string line;
  long curBlockId = -1;
  getline(infile,line);
  data *t = parse(line);
  if (t->id != "ID") infile.seekg(0);
  while (getline(infile,line)) {
    t = parse(line);
    if (t->id == "ID") continue; //header row
    if (t->lvl <= 1) {
      path = zeropath;
      curBlockId++;
    }
    path[t->lvl]++;
    for (int ip = t->lvl+1; ip < 13; ip++) path[ip] = '0';
    t->path = path;
    t->iblock = curBlockId;
    outfile << t->id << ','
	    << t->section << t->scheme << t->clas << '/' << t->group << ','
	    << t->section << ','
	    << t->scheme << ','
	    << t->clas << ','
	    << t->group << ','
	    << t->lvl;
    for (int inode = 0; inode < 13; inode++) outfile << ',' << t->path[inode];
    outfile << endl;
    delete t;
  }
  outfile.close();
  infile.close();
}

data* parse(string s) {
  istringstream iss(s);
  data *d = new data();
  getline(iss,d->id,',');
  getline(iss,d->cpc1,'/');
  getline(iss,d->group,',');
  getline(iss,d->level,',');
  istringstream(d->level) >> d->lvl;
  d->section = d->cpc1.substr(0,1);
  d->scheme = d->cpc1.substr(1,3);
  d->clas = d->cpc1.substr(4);
  return d;
}
