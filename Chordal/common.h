#ifndef COMMON_H_
#define COMMON_H_

#include <iostream>
#include <random>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <math.h> 
#include <time.h>
#include <set>
#include <list>
#include <lemon/list_graph.h>
#include <lemon/connectivity.h>
#include <lemon/lgf_writer.h>
#include <lemon/lgf_reader.h>
#include <lemon/kruskal.h>
#include <lemon/bfs.h>
#include <lemon/full_graph.h>
#include <lemon/adaptors.h>
#include <lemon/core.h>
#include <numeric>
#include <map>

using lemon::INVALID;
using namespace std;
using lemon::Bfs;

typedef lemon::ListGraph Graph;
typedef Graph::EdgeIt EdgeIt;
typedef Graph::IncEdgeIt IncEdgeIt;
typedef Graph::Edge Edge;
typedef Graph::NodeIt NodeIt;
typedef Graph::Node Node;
typedef Graph::EdgeMap<bool> EdgeMap;
typedef Graph::NodeMap<bool> NodeMap;
typedef Graph::NodeMap<int> CompMap;
typedef set<pair<int, int>>::iterator setPairIterator;
typedef set<int>::iterator setIterator;


double GetTimeUsage(clock_t clockEnd, clock_t clockBegin);

long long CalculateNChooseK(int n, int k);

long long Factorial(int n);

#endif