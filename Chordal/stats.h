#ifndef STATS_
#define STATS_

#include "chordal.h"

class MCSNode {
public:
	int _label, _numLabeledNeighbours;
	list<int>::iterator _location;
	int _unlabeledIndex;
	int _cliqueID;
};

class Stats {
public:
	Stats(ChordalGraph& chg);
	int GetNumNodes() const { return _numNodes; };
	int GetNumEdges() const { return _numEdges; };
	int GetNumConnComps() const { return _numConnComps; };
	int GetNumMaximalCliques() const { return _numMaximalCliques; };
	int GetMaxSize() const { return _maxSize; };
	int GetMinSize() const { return _minSize; };
	int GetAvgSize() const { return _avgSize; };
	double GetStdev() const { return _stdev; };
	double GetDensity() const { return _density; };
	const vector<int>& GetCliqueSizeFreqs() const { return _sizeFrequenciesOfMaximalCliques; }

	void CalculateStats();

private:
	ChordalGraph* _pchg;
	int _numNodes, _numEdges;
	double _density; //edge density of the graph

	int _numMaximalCliques, _numConnComps;
	int _maxSize, _minSize;
	double _avgSize, _stdev;

	vector<vector<int>> _maximalCliques;
	vector<int> _peo;
	vector<int> _sizeFrequenciesOfMaximalCliques;

	void ModifiedMCS();
	void CalculateCliqueSizeFrequencies();
	void CalculateStatsOfMaximalCliques();
};

#endif // !STATS_
