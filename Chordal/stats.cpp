#include "stats.h"

Stats::Stats(ChordalGraph& chg) {
	_pchg = &chg;
	const Graph& g = _pchg->GetGraph();
	_numNodes = countNodes(g);
	_numEdges = countEdges(g);
	_density = double(_numEdges) / (_numNodes*(_numNodes - 1) / 2);
	_numConnComps = countConnectedComponents(g);
	_maximalCliques.reserve(_numNodes);
	_stdev = _avgSize = _minSize = _maxSize = _numMaximalCliques = 0;
}


void Stats::CalculateStats() {
	ModifiedMCS();
	CalculateStatsOfMaximalCliques();
	CalculateCliqueSizeFrequencies();
}

/*
 * Maximum Cardinality Search method for chordal graphs
 */
void Stats::ModifiedMCS() {
	const Graph& g = _pchg->GetGraph();
	_maximalCliques.clear();
	vector<MCSNode> nodes;
	nodes.resize(_numNodes);
	vector<list<int>> unlabeledNodes;
	unlabeledNodes.resize(_numNodes);

	for (int i = 0; i < _numNodes; ++i) {
		unlabeledNodes.at(0).push_back(i); //initially all vertices are unlabeled (so their neighbours are unlabelled too)
	}
	list<int>::iterator it = unlabeledNodes.at(0).begin();
	for (int i = 0; i < _numNodes; ++i) {
		nodes.at(i)._location = it; //initial positions of unlabeled nodes
		++it;
		nodes.at(i)._label = -1;
		nodes.at(i)._unlabeledIndex = 0;
		nodes.at(i)._numLabeledNeighbours = 0;
	}
	int prevCard = 0, s = -1;
	int index = 0; //this will give the index of unlabeled nodes with maximum number of labeled neighbours
	for (int i = _numNodes - 1; i >= 0; --i) {
		int currentNodeID = unlabeledNodes.at(index).front(); // an unlabeled vertex with maximum number of labeled neighbours
		unlabeledNodes.at(index).pop_front(); //erase the selected node
		while (unlabeledNodes.at(index).size() < 0.5 && index > 0.5)
			index--;
		nodes.at(currentNodeID)._label = i;
		//update labels, indices, locations, and the number of labeled neighbours of each vertex
		Node currentNode = g.nodeFromId(currentNodeID);
		for (IncEdgeIt e(g, currentNode); e != INVALID; ++e) {
			int neighbourID = g.id(g.runningNode(e));
			if (nodes.at(neighbourID)._label < -0.5) { //if that neighbour is unlabeled
				nodes.at(neighbourID)._numLabeledNeighbours++;
				unlabeledNodes.at(nodes.at(neighbourID)._unlabeledIndex).erase(nodes.at(neighbourID)._location); //delete that node from that row 
				nodes.at(neighbourID)._unlabeledIndex++;
				if (nodes.at(neighbourID)._unlabeledIndex > index) index = nodes.at(neighbourID)._unlabeledIndex;
				list<int>::iterator it = unlabeledNodes.at(nodes.at(neighbourID)._unlabeledIndex).insert(unlabeledNodes.at(nodes.at(neighbourID)._unlabeledIndex).end(), neighbourID);
				nodes.at(neighbourID)._location = it;
			}
		}
		int newCard = nodes.at(currentNodeID)._numLabeledNeighbours;
		if (newCard <= prevCard) { //begin new clique
			s++;
			_maximalCliques.resize(s + 1);
			Node currentNode = g.nodeFromId(currentNodeID);
			for (IncEdgeIt e(g, currentNode); e != INVALID; ++e) {
				int nodeID = g.id(g.runningNode(e));
				if (nodes.at(nodeID)._label > -0.5) _maximalCliques.at(s).push_back(nodeID);
			}
			if (newCard != 0) {
				int minLabel = _numNodes + 1;
				for (unsigned int i = 0; i < _maximalCliques.at(s).size(); ++i) {
					if (nodes.at(_maximalCliques.at(s).at(i))._label < minLabel) {
						minLabel = nodes.at(_maximalCliques.at(s).at(i))._label;
					}
				}
			}
		}
		nodes.at(currentNodeID)._cliqueID = s;
		_maximalCliques.at(s).push_back(currentNodeID);
		prevCard = newCard;
	}
}

void Stats::CalculateCliqueSizeFrequencies() {
	for (unsigned int i = 0; i < _maximalCliques.size(); ++i) {
		unsigned int cliqueSize = _maximalCliques.at(i).size();
		if (_sizeFrequenciesOfMaximalCliques.size() < cliqueSize + 1) {
			// so that subtreeSize^th entry is guaranteed to exist (resize equates the newly added entries to zero)
			_sizeFrequenciesOfMaximalCliques.resize(cliqueSize + 1);
		}
		_sizeFrequenciesOfMaximalCliques.at(cliqueSize)++;
	}
}

void Stats::CalculateStatsOfMaximalCliques() {
	_maxSize = 0;
	_minSize = 9999999;
	_numMaximalCliques = _maximalCliques.size();
	_avgSize = 0;
	long long int sumSquaredSizes = 0;
	for (int i = 0; i < _numMaximalCliques; ++i) {
		int size = _maximalCliques.at(i).size();
		_avgSize += size;
		sumSquaredSizes += size*size;
		if (_maxSize < size) _maxSize = size;
		if (_minSize > size) _minSize = size;
	}
	_avgSize = double(_avgSize) / _numMaximalCliques;
}
