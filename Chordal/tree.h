#ifndef TREE_H_
#define TREE_H_

#include "common.h"
#include "global.h"

class Tree {
public:
	int _numNodes;
	Graph _g;
	vector<Edge> _treeVector;
	vector<double> _probVector;
	vector<set<int>> _subtreesS;
	Tree(int numNodes)	{ _numNodes = numNodes;	}
	void BuildRandomTree();
	void FindAndAddGrowingSubtree(int subtreeSize);
	void FindAndAddSubtreeWithEdgeDeletion(double deletionPercentage, double selectionBarrier);
	void FindAndAddSubtreeByConnNodes(int numNodesToConnect, vector<int>& allNodeIDs);

private:
	void SetProbsToEdges();
	void SetProbsToNodes();
	void ShuffleEdgeIDs(vector<int>& shuffledEdgeIDs);
	void FindNodesInRandomlySelectedConnComp(const Graph& g, set<int>& subtree, double selectionBarrier);
	void RevealSubtreeOnSelectedNodes(const vector<int>& allNodeIDs, int separator, set<int>& subtree);
};


class SearchNode {
public:
	vector<int> _admissibleNodeIDs;
	bool _admissibleVectorConstructed;
	SearchNode(int nodeID);
	void AddAllAdmissibleNodes(const Graph& g, vector<bool>& labels);
private:
	int _nodeID;

};

#endif
