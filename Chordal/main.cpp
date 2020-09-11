#include "chordal.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(a,b) _mkdir(a)
#define DIRECTORY_SEPARATOR "\\"
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#define DIRECTORY_SEPARATOR "/"
#endif

#define OUT_BASE_DIR "C:\\Users\\TheUser\\Desktop\\"
#define MAIN_FOLDER "ChordalGraphs"
#define INSTANCE_INFO_FILE_NAME "instance_info.csv"
#define CLIQUE_SIZE_INFO_FILE_NAME	"clique_size_frequencies.csv"

string method;				// Parameter #1: Subtree generation method (gs, cn, pt, or peo)
int n;						// Parameter #2: Number of vertices

int maxSubtreeSize;			// Parameter #3 (gs): Maximum subtree size for the growingSUbtree method

double thresholdProb;		// Parameter #3 (cn): Threshold probability value for the connectingNodes method

double edgeDelPerc;			// Parameter #3 (pt): Edge deletion percentage for the prunedTree method
double barrier;				// Parameter #4 (pt): Selection barrier quantile for subtree selection in the prunedTree method

double upperBoundCoef;		// Parameter #3 (peo): "Upper bound coefficient" parameter (used in Seker et al. (2018)) for the peo-based method of Andreou et al. (2005)

char *graphIndex;           // Parameter #4/5: The index of the graph


static ChordalGenerationParams readArguments(int argc, char ** argv) {
	if (argc < 5 || argc > 6) {
		cerr << "Usage: " << argv[0] << " chordal_gen_method number_of_vertices method_specific_param(s) graph_index" << endl;
		cerr << "Options for chordal graph generation method: \n" << "gs: growingSubtree \n" << "cs: connectingNodes \n" << "pt: prunedTree \n" << "peo: peoBased" << endl;
		cerr << "growingSubtree-specific params: max_subtree_size (integer between 1 and number_of_vertices" << endl;
		cerr << "connectingNodes-specific params: node_sel_prob_param (double in (0,1])" << endl;
		cerr << "prunedTree-specific params: edge_deletion_perc (double in [0,1)) and selection_barrier (double in [0,1))" << endl;
		cerr << "prunedTree-specific params: edge_deletion_perc (double in [0,1)) and selection_barrier (double in [0,1))" << endl;
		cerr << "peoBased-specific params: upper_bound_coef (double in (0,1]), the larger its value the higher the density tends to" << endl;
		
		system("pause");

		exit(EXIT_FAILURE);
	}

	method = argv[1];

	ChordalGenerationParams params(method);
	
	if (method == "gs" || method == "cn" || method == "pt" || method == "peo") {
		
		n = stoi(argv[2]);
		params._numNodes = n;

		if (n < 1)	{
			cout << "n should be a positive integer!" << endl;
			exit(EXIT_FAILURE);
		}
		if (method == "gs") {
			maxSubtreeSize = stoi(argv[3]);
			if (maxSubtreeSize < 1) {
				cout << "max_subtree_size should be a positive integer!" << endl;
				exit(EXIT_FAILURE);
			}
			params._maxSubtreeSize = maxSubtreeSize;
			graphIndex = argv[4];
		} else if (method == "cn") {
			thresholdProb = stod(argv[3]);
			if (thresholdProb > 1 || thresholdProb <= 0) {
				cout << "node_sel_prob_param should be in (0,1]!" << endl;
				exit(EXIT_FAILURE);
			}
			params._thresholdProb = thresholdProb;
			graphIndex = argv[4];
		} else if (method == "pt") {
			edgeDelPerc = stod(argv[3]);
			barrier = stod(argv[4]);
			if (edgeDelPerc >= 1 || edgeDelPerc < 0) {
				cout << "edge_deletion_perc should be in [0,1)!" << endl;
				exit(EXIT_FAILURE);
			}
			if (barrier >= 1 || barrier < 0) {
				cout << "selection_barrier should be in [0,1)!" << endl;
				exit(EXIT_FAILURE);
			}
			params._edgeDelPerc = edgeDelPerc;
			params._barrier = barrier;
			graphIndex = argv[5];
		} else if (method == "peo") {
			upperBoundCoef = stod(argv[3]);
			if (upperBoundCoef > 1 || upperBoundCoef <= 0) {
				cout << "upper_bound_coef should be in (0,1]!" << endl;
				exit(EXIT_FAILURE);
			}
			params._upperBoundCoef = upperBoundCoef;
			graphIndex = argv[4];
		}
	} else {
		cout << "Invalid method selection. Select 'gs', 'cn', 'pt', or 'peo'." << endl;
		exit(EXIT_FAILURE);
	}

	return params;
}


int main(int argc, char *argv[]) {
	cout << "Starting to read arguments..." << endl;
	ChordalGenerationParams params = readArguments(argc, argv);
	cout << "Finished reading arguments..." << endl;
	
	ChordalGraph* pchg;
	pchg = new ChordalGraph(params);
	string methodFolder = params._method;
	pchg->Build();

	// Write graph to file (in LEMON's graph format .lgf)
	string mainFolder = MAIN_FOLDER;
	string outDir = OUT_BASE_DIR + mainFolder;
	mkdir(outDir.c_str(), 0777);
	string outDirMethod = outDir + DIRECTORY_SEPARATOR + methodFolder + DIRECTORY_SEPARATOR;
	mkdir(outDirMethod.c_str(), 0777);
	string outDirInst = outDirMethod + to_string(n) + DIRECTORY_SEPARATOR;
	mkdir(outDirInst.c_str(), 0777);
	string fileName = outDirInst + "chordalgr_" + to_string(n) + "_" + graphIndex + ".lgf";
	pchg->WriteGraphToFile(fileName);

	// Write instance stats to file
	string instanceInfoFileName = outDirMethod;
	instanceInfoFileName = instanceInfoFileName + DIRECTORY_SEPARATOR + INSTANCE_INFO_FILE_NAME;
	string cliqueSizeInfoFileName = outDirMethod;
	cliqueSizeInfoFileName = cliqueSizeInfoFileName + DIRECTORY_SEPARATOR + CLIQUE_SIZE_INFO_FILE_NAME;
	pchg->CollectAndWriteStats(fileName, instanceInfoFileName, cliqueSizeInfoFileName);

	delete pchg; 

}
