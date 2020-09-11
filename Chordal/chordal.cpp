#include "chordal.h"
#include "tree.h"
#include "stats.h"


ChordalGraph::ChordalGraph(ChordalGenerationParams params) :_peoBased(*this), _subtreeIntersection(*this) {
	_method = params._method;
	_numNodes = params._numNodes;
	_maxSubtreeSize = params._maxSubtreeSize;
	_thresholdProb = params._thresholdProb;
	_edgeDelPerc = params._edgeDelPerc;
	_barrier = params._barrier;
	_upperBoundCoef = params._upperBoundCoef;
	_timeToBuild = 0;
}

void ChordalGraph::Build() {
	clock_t begin = clock();
	if (_method == "peo") {
		_peoBased.Build();
	} else {
		_subtreeIntersection.Build();
	}
	clock_t end = clock();
	_timeToBuild = double(GetTimeUsage(end, begin)) / 1000;
}

void ChordalGraph::CollectAndWriteStats(string fileName, string instanceInfoCSVFileName, string cliqueSizeInfoCSVFileName) {

	Stats* stats;
	stats = new Stats(*this);
	stats->CalculateStats();

	ofstream file;
	file.open(instanceInfoCSVFileName, ios::out | ios::ate | ios::app);
	file << fileName << ",";

	string methodName;
	if (_method == "gs") {
		methodName = "growingSubtree";
		file << methodName << "," << _maxSubtreeSize << ",";
	} else if (_method == "cn") {
		methodName = "connectingNodes";
		file << methodName << "," << _thresholdProb << ",";
	} else if (_method == "pt") {
		methodName = "prunedTree";
		file << methodName << "," << _edgeDelPerc << "," << _barrier << ",";
	} else if (_method == "peo") {
		methodName = "peoBased";
		file << methodName << "," << _upperBoundCoef << ",";
	}

	file << stats->GetNumNodes() << "," << stats->GetDensity() << "," << stats->GetNumEdges() << "," << stats->GetNumConnComps() << ","
			<< stats->GetNumMaximalCliques() << "," << stats->GetMaxSize() << "," << stats->GetMinSize() << "," << stats->GetAvgSize() << ","
			<< stats->GetStdev() << "," << _timeToBuild << endl;
	file.close();


	cout << "A chordal graph with " << _numNodes << " nodes and " << stats->GetDensity() << " density is produced." << endl;

	const vector<int>& cliqueSizeFreqs = stats->GetCliqueSizeFreqs();
	ofstream file2;
	file2.open(cliqueSizeInfoCSVFileName, ios::out | ios::ate | ios::app);
	file2 << fileName << ",";
	for (unsigned int i = 1; i < cliqueSizeFreqs.size(); ++i) //0th entry is not needed because the indices correspond to clique sizes
		file2 << cliqueSizeFreqs.at(i) << ",";
	file2 << endl;
	file2.close();

	delete stats;
}

void ChordalGraph::SubtreeIntersection::Build() {
	if (_chg._method == "gs") {
		BuildwithGrowingSubtree();
	} else if (_chg._method == "cn") {
		BuildwithConnectingNodes();
	} else if (_chg._method == "pt") {
		BuildwithPrunedTree();
	}
}

bool ChordalGraph::SubtreeIntersection::CheckIntersection(const vector<set<int>>& subtrees, int subtree1Index, int subtree2Index) {
	for (setIterator it1 = subtrees.at(subtree1Index).begin(); it1 != subtrees.at(subtree1Index).end(); ++it1) {
		if (subtrees.at(subtree2Index).find(*it1) != subtrees.at(subtree2Index).end()) //then they intersect
			return true;
	}
	return false;
}

void ChordalGraph::SubtreeIntersection::ConstructIntersectionGraph(const vector<set<int>>& subtrees) {
	//_g is an empty graph to be filled
	int numIntersections = 0;
	for (unsigned int i = 0; i < subtrees.size(); ++i) _chg._g.addNode();
	for (unsigned int i = 0; i < subtrees.size() - 1; ++i) {
		for (unsigned int j = i + 1; j < subtrees.size(); ++j) {
			if (CheckIntersection(subtrees, i, j)) {
				_chg._g.addEdge(_chg._g.nodeFromId(i), _chg._g.nodeFromId(j));
				numIntersections++;
			}
		}
	}
}

void ChordalGraph::SubtreeIntersection::BuildwithGrowingSubtree() {
	Tree tr(_chg._numNodes);
	tr.BuildRandomTree();
	for (int i = 0; i < _chg._numNodes; ++i) {
		uniform_int_distribution<int>  selector(1, _chg._maxSubtreeSize);
		int subtreeSize = selector(generator);
		tr.FindAndAddGrowingSubtree(subtreeSize);
	}
	ConstructIntersectionGraph(tr._subtreesS);
}

void ChordalGraph::SubtreeIntersection::BuildwithConnectingNodes() {
	Tree tr(_chg._numNodes);
	tr.BuildRandomTree();
	vector<int> allNodeIDs;
	for (int i = 0; i < _chg._numNodes; ++i)
		allNodeIDs.push_back(i);
	double lambda = _chg._thresholdProb*_chg._numNodes;
	for (int i = 0; i < _chg._numNodes; ++i) {
		std::poisson_distribution<int> selector(lambda);
		int numNodesToConnect = selector(generator);
		if (numNodesToConnect < 0.5) {
			numNodesToConnect = 1;
		} else if (numNodesToConnect > _chg._numNodes + 0.1) {
			numNodesToConnect = _chg._numNodes;
		}
		tr.FindAndAddSubtreeByConnNodes(numNodesToConnect, allNodeIDs);
	}
	ConstructIntersectionGraph(tr._subtreesS);
}

void ChordalGraph::SubtreeIntersection::BuildwithPrunedTree() {
	Tree tr(_chg._numNodes);
	tr.BuildRandomTree();
	for (int i = 0; i < _chg._numNodes; ++i) {
		tr.FindAndAddSubtreeWithEdgeDeletion(_chg._edgeDelPerc, _chg._barrier);
	}
	ConstructIntersectionGraph(tr._subtreesS);
}



void ChordalGraph::PEOBasedConstruction::Build() {
	BuildPEOBased();
}

void ChordalGraph::PEOBasedConstruction::BuildPEOBased()
{
	_adjList.resize(_chg._numNodes);

	int lowerBound = 1;
	for (int i = 0; i < _chg._numNodes - 1; ++i) {
		int upperBound = ceil((_chg._numNodes - i - 1)*_chg._upperBoundCoef);
		uniform_int_distribution<int> neighCount(lowerBound, upperBound);
		int numNeighbours = neighCount(generator);
		if (numNeighbours < _chg._numNodes - i - 1) { // if there is room for random selection, then do it
			set<int> neighbourhood;
			neighbourhood.insert(i); //neighbourhood includes the node itself
			AssignSuccesiveNeighbours(neighbourhood, numNeighbours, i);
			FindFormerSuccessiveNeighbours(i, neighbourhood);
			MakeNeighbourhoodClique(neighbourhood);
		} else {
			MakeTheRestClique(i);
			break;
		}
	}
	ConvertToLemonObject();
}

void ChordalGraph::PEOBasedConstruction::AssignSuccesiveNeighbours(set<int>& neighbourhood, int numNeighbours, int nodeID) {
	while ((int) neighbourhood.size() < numNeighbours + 1) {
		uniform_int_distribution<int> neighPlaces(nodeID + 1, _chg._numNodes - 1);
		int neighbourID = neighPlaces(generator);
		neighbourhood.insert(neighbourID);
	}
}

void ChordalGraph::PEOBasedConstruction::FindFormerSuccessiveNeighbours(int nodeID, set<int>& neighbours) {
	setIterator it = _adjList.at(nodeID).lower_bound(nodeID + 1);
	for (; it != _adjList.at(nodeID).end(); ++it) {
		neighbours.insert(*it);
	}
}

void ChordalGraph::PEOBasedConstruction::MakeNeighbourhoodClique(const set<int>& neighbourhood) {
	int edgeCounter = 0;
	for (setIterator i = neighbourhood.begin(); i != neighbourhood.end(); ++i) {
		for (setIterator j = neighbourhood.begin(); j != neighbourhood.end(); ++j) {
			if ((*i) != (*j)) { //no self-loops allowed
				if (_adjList.at(*i).insert(*j).second) edgeCounter++;
			}
		}
	}
	edgeCounter = edgeCounter / 2; //since each edge is counted twice above
	_numEdges += edgeCounter;
}

void ChordalGraph::PEOBasedConstruction::MakeTheRestClique(int nodeID) {
	int edgeCounter = 0;
	for (int i = nodeID; i < _chg._numNodes; ++i) {
		for (int j = nodeID; j < _chg._numNodes; ++j) {
			if (i != j) {
				if (_adjList.at(i).insert(j).second) edgeCounter++;
			}
		}
	}
	edgeCounter = edgeCounter / 2; //since each edge is counted twice above
	_numEdges += edgeCounter;
}

void ChordalGraph::PEOBasedConstruction::ConvertToLemonObject()
{
	for (int i = 0; i < _chg._numNodes; ++i) _chg._g.addNode();
	for (int i = 0; i < _chg._numNodes - 1; ++i) {
		Node u = _chg._g.nodeFromId(i);
		setIterator it = _adjList.at(i).lower_bound(i + 1);
		for (; it != _adjList.at(i).end(); ++it) {
			Node v = _chg._g.nodeFromId(*it);
			_chg._g.addEdge(u, v);
		}
	}
}
