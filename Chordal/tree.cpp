#include "tree.h"

SearchNode::SearchNode(int nodeID) {
		_nodeID = nodeID;
		_admissibleVectorConstructed = false;
}

void SearchNode::AddAllAdmissibleNodes(const Graph& g, vector<bool>& labels) {
	Node n = g.nodeFromId(_nodeID);
	for (IncEdgeIt e(g, n); e != INVALID; ++e) {
		int candidateID = g.id(g.runningNode(e));
		if (!labels.at(candidateID)) //if that arc is admissible
			_admissibleNodeIDs.push_back(candidateID);
	}
	_admissibleVectorConstructed = true;
}

bool PairComparison(pair<int, double> p1, pair<int, double> p2) {
	return (p1.second < p2.second);
}

bool PairComparison2(pair<int, vector<int>> p1, pair<int, vector<int>> p2) {
	return (p1.first < p2.first);
}

void Tree::BuildRandomTree() {
	_g.addNode();
	for (int i = 1; i < _numNodes; ++i) {
		uniform_int_distribution<int>  nodeSelector(0, i - 1);
		int selectedNodeID = nodeSelector(generator); 
		Node u = _g.nodeFromId(selectedNodeID);
		Node v = _g.addNode();
		_g.addEdge(u, v);
	}
}

void Tree::SetProbsToEdges() {
	_probVector.clear();
	for (int i = 0; i < _numNodes - 1; ++i) {
		_probVector.push_back(((double)rand()) / ((long) RAND_MAX+1));
	}
}

void Tree::SetProbsToNodes() {
	_probVector.clear();
	for (int i = 0; i < _numNodes; ++i)	{
		_probVector.push_back(double(rand()) / ((long) RAND_MAX + 1));
	}
}

void Tree::ShuffleEdgeIDs(vector<int>& shuffledEdgeIDs) {
	int numEdges = _numNodes - 1;
	vector<pair<int, double>> edgeOrder;
	for (int i = 0; i < numEdges; ++i) 	{
		pair<int, double> p(i, double(rand()) / ((long) RAND_MAX + 1));
		edgeOrder.push_back(p);
	}
	sort(edgeOrder.begin(), edgeOrder.end(), PairComparison);
	for (int i = 0; i < numEdges; ++i) 	{
		shuffledEdgeIDs.push_back(edgeOrder.at(i).first);
	}
}

void Tree::FindNodesInRandomlySelectedConnComp(const Graph& g, set<int>& subtree, double selectionBarrier) {
	vector<vector<Node>> connComps;
	CompMap compMap(g);
	int numConnComps = connectedComponents(g, compMap);
	connComps.resize(numConnComps);
	for (NodeIt n(g); n != INVALID; ++n)
		connComps.at(compMap[n]).push_back(n);
	vector<pair<int, vector<int>>> sizesAndCompIDs; //first entry of the pair is the size, and the second entry is the vector of comp IDs with that size
	int sumOfDistinctCompSizes = 0;
	for (unsigned int i = 0; i < connComps.size(); ++i) {
		int index = -1;
		for (unsigned int j = 0; j < sizesAndCompIDs.size(); ++j) {
			if (connComps.at(i).size() == (unsigned int) sizesAndCompIDs.at(j).first) { //if that size is already encountered
				index = j;
				break;
			}
		}
		if (index == -1) { //if no such size is encountered before
			sizesAndCompIDs.resize(sizesAndCompIDs.size() + 1);
			sizesAndCompIDs.back().first = connComps.at(i).size();
			sizesAndCompIDs.back().second.push_back(i); //i is the ID of the connected component
			sumOfDistinctCompSizes += connComps.at(i).size();
		} else 	{
			sizesAndCompIDs.at(index).first = connComps.at(i).size();
			sizesAndCompIDs.at(index).second.push_back(i); //i is the ID of the connected component
		}
	}
	sort(sizesAndCompIDs.begin(), sizesAndCompIDs.end(), PairComparison2); //to sort component sizes in ascending order
	int numDifferentCompSizes = sizesAndCompIDs.size();
	int firstIndex = floor(selectionBarrier*numDifferentCompSizes);
	uniform_int_distribution<int>  selector(firstIndex, numDifferentCompSizes - 1);
	int index = selector(generator);
	uniform_int_distribution<int>  selector2(0, sizesAndCompIDs.at(index).second.size() - 1);
	int selectedCompID = sizesAndCompIDs.at(index).second.at(selector2(generator));
	for (unsigned int i = 0; i < connComps.at(selectedCompID).size(); ++i) {
		subtree.insert(_numNodes - 1 - g.id(connComps.at(selectedCompID).at(i))); //since graphCopy fnc reverses node IDs, we insert the IDs like this
	}
}

void Tree::RevealSubtreeOnSelectedNodes(const vector<int>& allNodeIDs, int separator, set<int>& subtree) {
		if (separator > 1.1) { //if there are more than 1 node selected
			Bfs<Graph> bfs(_g);
			for (int i = 1; i < separator; ++i) {
				Node currentNode = _g.nodeFromId(allNodeIDs.at(i));
				bfs.run(_g.nodeFromId(allNodeIDs.at(0)), currentNode);
				Node prevNode = bfs.predNode(_g.nodeFromId(allNodeIDs.at(i)));
				subtree.insert(allNodeIDs.at(i));
				while (prevNode != INVALID) {
					int prevNodeID = _g.id(prevNode);
					subtree.insert(prevNodeID);
					currentNode = prevNode;
					prevNode = bfs.predNode(currentNode);
				}
			}
		} else {
			subtree.insert(allNodeIDs.at(0));
		}
	}
/*
 * To grow the subtree, e select a random node from it, and then randomly select an admissible neighbour of that node
 */
void Tree::FindAndAddGrowingSubtree(int subtreeSize) {
	set<int> subtree;
	uniform_int_distribution<int>  selector(0, _numNodes - 1);
	int startNodeID = selector(generator); //select a random node on tree to start with
	subtree.insert(startNodeID);
	if (subtreeSize > 1.5) {
		vector<bool> labels; //false will denote unmarked
		labels.resize(_numNodes);
		for (int i = 0; i < _numNodes; ++i) labels.at(i) = false;
		labels.at(startNodeID) = true;
		vector<SearchNode> list;
		SearchNode n(startNodeID);
		n.AddAllAdmissibleNodes(_g, labels);
		list.push_back(n);
		int currentSubtreeSize = subtree.size(); //=1 now
		int pickedIndex = 0;
		while (currentSubtreeSize < subtreeSize) {
			uniform_int_distribution<int>  selector2(0, list.at(pickedIndex)._admissibleNodeIDs.size() - 1);
			int selectedAdmissibleNodeIndex = selector2(generator); 
			int selectedAdmissibleNodeID = list.at(pickedIndex)._admissibleNodeIDs.at(selectedAdmissibleNodeIndex);
			SearchNode n2(selectedAdmissibleNodeID);
			list.push_back(n2);
			//remove the selected admissible node from the admissibles list
			list.at(pickedIndex)._admissibleNodeIDs.erase(list.at(pickedIndex)._admissibleNodeIDs.begin() + selectedAdmissibleNodeIndex);
			labels.at(selectedAdmissibleNodeID) = true;
			subtree.insert(selectedAdmissibleNodeID);
			currentSubtreeSize++;
			uniform_int_distribution<int>  selector3(0, list.size() - 1);
			pickedIndex = selector3(generator); //pick a random node from the list
			if (!list.at(pickedIndex)._admissibleVectorConstructed) {
				list.at(pickedIndex).AddAllAdmissibleNodes(_g, labels);
			}
			int numAdmissibleNodes = list.at(pickedIndex)._admissibleNodeIDs.size();
			while (numAdmissibleNodes == 0 && currentSubtreeSize < subtreeSize) { //pick a node from list which has an admissible node
				list.erase(list.begin() + pickedIndex);
				uniform_int_distribution<int>  selector4(0, list.size() - 1);
				pickedIndex = selector4(generator); 
				if (!list.at(pickedIndex)._admissibleVectorConstructed) {
					list.at(pickedIndex).AddAllAdmissibleNodes(_g, labels);
				}
				numAdmissibleNodes = list.at(pickedIndex)._admissibleNodeIDs.size();
			}
		}
	}
	_subtreesS.push_back(subtree);
}

/*
 * this fnc will randomly select deletionPercentage of edges from the tree and delete it
 */
void Tree::FindAndAddSubtreeWithEdgeDeletion(double deletionPercentage, double selectionBarrier) {
	Graph s;
	graphCopy(_g, s).run();
	int numEdgesToDelete = deletionPercentage*(_numNodes - 1); 
	vector<int> shuffledEdgeIDs;
	ShuffleEdgeIDs(shuffledEdgeIDs);
	for (int i = 0; i < numEdgesToDelete; ++i) {
		s.erase(s.edgeFromId(shuffledEdgeIDs.at(i)));
	}
	set<int> subtree;
	FindNodesInRandomlySelectedConnComp(s, subtree, selectionBarrier);
	_subtreesS.push_back(subtree);
}

/*
 * take numNodesToConnect-many nodes and connect them with the paths that connect them on the host tree to get a subtree
 */
void Tree::FindAndAddSubtreeByConnNodes(int numNodesToConnect, vector<int>& allNodeIDs) {
	int separator = 0; //separator takes the value of the element that it is on the left of
	while (separator < numNodesToConnect) {
		uniform_int_distribution<int>  selector(separator, _numNodes - 1);
		int selectedIndex = selector(generator);
		//swap the elements
		int temp = allNodeIDs.at(selectedIndex);
		allNodeIDs.at(selectedIndex) = allNodeIDs.at(separator);
		allNodeIDs.at(separator) = temp;
		separator++;
	}
	set<int> subtree;
	RevealSubtreeOnSelectedNodes(allNodeIDs, separator, subtree);
	_subtreesS.push_back(subtree);
}
