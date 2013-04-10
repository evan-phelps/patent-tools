#include <time.h>
#include <iostream>
#include <array>
#include <iomanip>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

using namespace std;

/* *****************************************************************
   ****************** REQUIRES C++11 !!!!!!!!!! ********************
   ************** g++ -std=c++11 score.cpp -o score ****************
   ***************************************************************** */
/* input file format: id,cpc,level (int,string,int)
   cpc: Xsssc[*]/gg[*] 
        where X = section, sss = scheme, c[*] = class, gg[*] = group
*/

typedef unordered_multimap< string,array<unsigned char,13> > lgs;

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
int blockend(int itarget, vector<data*> *targets);
int score(int itarget, int iquery, vector<data*> *targets, vector<lgs*> *lgblocks);
int match(int itarget,int iquery,vector<data*> *targets, int n);

int main(int argc, char* argv[])
{
  const int MAX = 66500;
  array<unsigned char,13> zeropath;
  array<unsigned char,13> path;
  zeropath.fill('0');
  clock_t t0, t1, t2;
  ifstream infile(argv[1]);
  ofstream outfile(argv[2]);

  unsigned char **scores = new unsigned char*[MAX];
  for (int i = 0; i < MAX; i++) scores[i] = new unsigned char[MAX];

  vector<data*> targets;
  vector<lgs*> lgblocks;
  long scoredist[21];
  for (int i = 0; i < 21; i++) scoredist[i] = 0;
  string line;
  outfile << "ID,CPC,Level";
  long curBlockId = -1;
  while (getline(infile,line)) {
    data *t = parse(line);
    if (t->lvl <= 1) {
      path = zeropath;
      curBlockId++;
      lgblocks.push_back(new lgs());
    }
    path[t->lvl]++;
    for (int ip = t->lvl+1; ip < 13; ip++) path[ip] = '0';
    t->path = path;
    t->iblock = curBlockId;
    lgblocks[curBlockId]->insert(lgs::value_type(t->group,t->path));
    targets.push_back(t);
    outfile << ',' << t->section << t->scheme << t->clas << '/' << t->group;
    //for(auto& s: t->path)  cout << s << ',';
    //cout << endl;
  }
  size_t size = targets.size();
  cout << "Section " << targets[0]->section << " -- " << size << " records" << endl;
  t0 = clock();
  t1 = clock();
  for (int itarget = 0; itarget < size; itarget++) {
    data *t = targets[itarget];
    scores[itarget][itarget] = 200;
    scoredist[20]++;
    if (itarget+1 < size) {
      for (int iquery = itarget+1; iquery < targets.size(); iquery++) {
	int iscore = score(itarget,iquery,&targets,&lgblocks);
	scores[itarget][iquery] = scores[iquery][itarget] = iscore;
	int idx = (int)(iscore/10);
	if (idx >= 0) scoredist[idx] += 2;
	else cout << "WARNING! score = " << iscore << " for query id = " << targets[iquery]->id << ", target id = " <<  targets[itarget]->id << endl;
      }
    }
    if ((itarget+1)%1000==0) {
      t2 = clock();
      double blocktime = (double)(t2-t1)/CLOCKS_PER_SEC;
      double totaltime = (double)(t2-t0)/CLOCKS_PER_SEC;
      double fraction = (double)(itarget+1)/size;
      double remaining = (totaltime/fraction-totaltime)/60.0;
      cout << "processed " << itarget+1 << " (" << setprecision(2) << 100*fraction << "%); block time = " << blocktime/60  << " minutes ... " << remaining  << " minutes remaining" << endl;
      t1 = clock();
    }
  }
  t2 = clock();
  double totaltime = (double)(t2-t0)/CLOCKS_PER_SEC;
  cout << "finished!\nTotal time: " << setprecision(1) << totaltime/60 << " minutes." << endl;
  cout << "score distribution" << endl;
  long counts = 0;
  for (int i = 0; i < 21; i++) {
    cout << i*10 << "\t" << scoredist[i] << endl;
    counts += scoredist[i];
  }
  cout << "*********************" << endl;
  cout << "counts = " << counts << " : expected = " << size*size << endl;

  for (int i = 0; i < size; i++) {
    data *t = targets[i];
    outfile << endl;
    outfile << t->id << ',' << t->section << t->scheme << t->clas 
        << "/" << t->group << ","  << t->lvl;
    for (int j = 0; j < size; j++) {
      outfile << "," << (int)scores[i][j];
    }
  }
  outfile << endl;
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

int blockend(int itarget, int n, vector<data*> *targets)
{
  size_t size = targets->size();
  data *t = (*targets)[itarget];
  int targetlvl = t->lvl;
  if (targetlvl==0) return itarget;
  int it = -1;
  for (it = itarget+1; (it < size) && (t = (*targets)[it]) && (t->lvl > 1 && t->lvl >= n); it++) ;
  return it-1;
}

int blockbegin(int itarget, int n, vector<data*> *targets)
{
  if (itarget <= 0) return 0;
  data *t = (*targets)[itarget];
  int targetlvl = t->lvl;
  if (targetlvl<=1) return itarget;
  int it = itarget-1;
  t = (*targets)[it];
  if (n==1) {
    while (t->lvl > n && --it >= 0) {
      t = (*targets)[it];
    }
    --it;
  } else {
    while (t->lvl >= n && --it >= 0) {
      t = (*targets)[it];
    }
  }
  return (it < -1) ? 0 : it+1;
}

int match(int itarget, int iquery, vector<data*> *targets, int n) {
  bool match = false;
  int lvl = -1;
  int it = blockbegin(itarget,n,targets);
  int max = blockend(itarget,n,targets);
  while (it <= max) {
    data *t = (*targets)[it];
    data *q = (*targets)[iquery];
    if ( t->group == q->group ) return true;
    it++;
  }
  return match;
}

int score(int itarget,int iquery,vector<data*> *targets, vector<lgs*> *lgblocks) {
  int ret = -1;
  int lvl = -1;
  data *t = (*targets)[itarget];
  data *q = (*targets)[iquery];
  if (t->scheme == q->scheme) {
    if (t->clas == q->clas) {
      if (t->group == q->group) ret = 200;
      else {
	auto its = (*lgblocks)[t->iblock]->equal_range(q->group);
	int deepestlvl = 1;
	int tlvl = t->lvl;
	bool match = false;
	for (auto it = its.first; it != its.second; it++) {
	  match = true;
	  if (tlvl < deepestlvl) break;
	  while ( tlvl >= deepestlvl && t->path[deepestlvl] == it->second[deepestlvl]) deepestlvl++;
	}
	if (tlvl>=deepestlvl && match) deepestlvl++;
	return 60+10*deepestlvl;
      }
    } else ret = 60;
  } else {
    if (t->scheme.compare(0,1,q->scheme,0,1) == 0) {
      if (t->scheme.compare(1,1,q->scheme,1,1) == 0) ret = 30;
      else ret = 20;
    } else ret = 10;
  }
  return ret;
}
