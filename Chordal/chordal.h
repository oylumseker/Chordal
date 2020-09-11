#ifndef CHORDALGRAPH_H_
#define CHORDALGRAPH_H_

#include "common.h"
#include "global.h"


class ChordalGenerationParams {
public:
	string _method = "";
	int _numNodes = -1;
	int _maxSubtreeSize = -1;                                    //param for growingSubtree (in terms of the number of nodes)
	double _thresholdProb = -1;                                  //param for connectingNodes
	double _edgeDelPerc = -1, _barrier = -1;                     //params for prunedTree
	double _upperBoundCoef = -1; 				     //params for peoBased

	ChordalGenerationParams(string method) : _method(method) {}
};


class ChordalGraph {
public:
	ChordalGraph(ChordalGenerationParams params);
	void Build();
	double GetBuildTime() const { return _timeToBuild; }
	void CollectAndWriteStats(string fileName, string instanceInfoCSVFileName, string cliqueSizeInfoCSVFileName);
	void WriteGraphToFile(string fileName) { graphWriter(_g, fileName).run();}
	const Graph& GetGraph() { return _g; }

private:
	class SubtreeIntersection {
	public:
		SubtreeIntersection(ChordalGraph& chg) : _chg(chg) {}
		void Build();
	private:
		ChordalGraph& _chg;
		void BuildwithGrowingSubtree();
		void BuildwithPrunedTree();
		void BuildwithConnectingNodes();
		bool CheckIntersection(const vector<set<int>>& subtrees, int subtree1Index, int subtree2Index);
		void ConstructIntersectionGraph(const vector<set<int>>& subtrees);
	};

	class PEOBasedConstruction {
	public:
		PEOBasedConstruction(ChordalGraph& chg): _chg(chg) {}
		void Build();
	private:
		ChordalGraph& _chg;
		vector<set<int>> _adjList;
		vector<int> _peo;
		int _numEdges = 0;
		void BuildPEOBased();
		void AssignSuccesiveNeighbours(set<int>& neighbourhood, int numNeighbours, int nodeID);
		void FindFormerSuccessiveNeighbours(int nodeID, set<int>& neighbours);
		void MakeNeighbourhoodClique(const set<int>& neighbourhood);
		void MakeTheRestClique(int nodeID);
		void ConvertToLemonObject();
	};


	int _numNodes;
	Graph _g;
	int _maxSubtreeSize;                            //param for growingSubtree (in terms of the number of nodes)
	double _thresholdProb;                          //param for connectingNodes
	double _edgeDelPerc, _barrier;                  //params for prunedTree
	double _upperBoundCoef; 			//param for peoBased
	double _timeToBuild;
	string _method;
	PEOBasedConstruction _peoBased;
	SubtreeIntersection _subtreeIntersection;

};


#endif
