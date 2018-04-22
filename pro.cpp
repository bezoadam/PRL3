/**
	autor: Adam Bezak xbezak01@stud.fit.vutbr.cz
	program: prirazeni poradi preorder vrcholum
*/

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm> 
#include <math.h>
#include <typeinfo>

using namespace std;

// #define DEBUG 1
#define TAG 0

/**
	Struktura hrany medzi dvoma vrcholmi
*/
struct Edge {
	int number;
	bool isForwardEdge;
	char start;
	char end;
};

/**
	Struktura sousednosti medzi dvoma vrcholmi
*/
struct AdjencyElement {
	struct Edge edge;
	struct Edge reverseEdge;
};

/**
	Struktura adjency listu
*/
struct AdjChar {
	char nodeChar;
	vector<struct AdjencyElement> adjencies;
};

/**
	Struktura pre pole suffix sum
*/
struct ProcEdge {
	int myEdgeNumber;
	int nextEdgeNumber;
	int suffixSumValue;
	bool isForwardEdge;
	char start;
	char end;
};

/**
    Pomocny ypis struktury hrany.

    @param struct Edge edge Hrana
    @return void
*/
void printStructEdge(struct Edge edge) {
	cout << "Number: " << edge.number << endl;
	cout << "Is forward edge " << edge.isForwardEdge << endl;
	cout << "Start symbol: " << edge.start << endl;
	cout << "End symbol: " << edge.end << endl;
}

/**
    Pomocny vypis struktury adjency listu.

    @param vector<struct AdjencyElement> adjencies adjency list
    @return void
*/
void printVectorOfAdjencies(vector<struct AdjencyElement> adjencies) {
	for(int i = 0; i < adjencies.size(); i++) {
		cout << '\n';
		cout << "Uzol c. " << i << endl;
		cout << "Edge:" << endl;
		printStructEdge(adjencies[i].edge);
		cout << "Edge reverse:" << endl;
		printStructEdge(adjencies[i].reverseEdge);
	}
}

/**
    Pomocny vypis struktury uzlov.

    @param vector<struct AdjChar> characterNodes Jednotlive uzly.
    @return void
*/
void printVectorCharacterNodes(vector<struct AdjChar> characterNodes) {
	for(int i = 0; i < characterNodes.size(); i++) {
		cout << '\n';
		cout << "Uzol symbol: " << characterNodes[i].nodeChar << endl;
		printVectorOfAdjencies(characterNodes[i].adjencies);
	}
}

/**
    Vypis suffix sum vektora.

    @param vector<struct ProcEdge> suffixSumResult Suffix sum vektor.
    @param *output Pointer na vypisujuci string
    @param treeSize velkost stromu pre korekciu
    @return void
*/
void printFinalVector(vector <struct ProcEdge> suffixSumResult, string *output, int treeSize) {
	for(int i =0; i < suffixSumResult.size(); i++) {
		if (suffixSumResult[i].isForwardEdge) {
			(*output)[treeSize - suffixSumResult[i].suffixSumValue] = suffixSumResult[i].end;
		}
	}
}

/**
    Zistenie nasledujucej hrany na zaklade aktualneho cisla hrany.

    @param vector<struct AdjChar> characterNodes Jednotlive uzly.
    @param int myEdgeNumber cislo zistovanej hrany
    @return struct ProcEdge
*/
struct ProcEdge ETour(vector<struct AdjChar> characterNodes, int myEdgeNumber) {
	struct ProcEdge procEdge = {
		-1,
		-1,
		-1,
		false,
		'X',
		'X'
	};
	for(int i = 0; i < characterNodes.size(); i++) {
		int leftSideIndex = 0;
		for(int j = 0; j < characterNodes[i].adjencies.size(); j++) {
			struct AdjencyElement element = characterNodes[i].adjencies[j];
			// Postupne si ukladam reverznu hranu ako nasledujucu
			if (element.edge.number == myEdgeNumber && procEdge.nextEdgeNumber == -1) {
				procEdge.nextEdgeNumber = element.reverseEdge.number;
				procEdge.isForwardEdge = element.edge.isForwardEdge;
				procEdge.start = element.edge.start;
				procEdge.end = element.edge.end;
			}
			if (element.reverseEdge.number == myEdgeNumber) {
				// V pripade, ze hladam na konci adjency listu, tak berem ako dalsiu hranu z prveho prvku
				if ((characterNodes[i].adjencies.size() - 1) == j) {
					procEdge.nextEdgeNumber = characterNodes[i].adjencies[0].edge.number;
					procEdge.start = element.reverseEdge.start;
					procEdge.end = element.reverseEdge.end;
				} else {
					// V opacnom pripade si ulozim nasleduju hranu z nasledujuceho prvku adjency listu
					procEdge.nextEdgeNumber = characterNodes[i].adjencies[j + 1].edge.number;
					procEdge.start = element.reverseEdge.start;
					procEdge.end = element.reverseEdge.end;
				}
			}
		}
	}
	return procEdge;
}

int main(int argc, char* argv[])
{
   	int numprocs, myId, myEdgeNumber, nextEdgeNumber;
	MPI_Status status;	//struct- obsahuje kod- source, tag, error
	string tree(argv[1]);
	vector<struct AdjChar> characterNodes;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // zistime pocet procesorov
	MPI_Comm_rank(MPI_COMM_WORLD, &myId);           // zistime id svojho procesoru

	int counter = 1;
	double startTime, endTime;
	startTime = MPI_Wtime();	//pociatocny cas

	if (myId == 0) {
		//vytvorenie adjency listu
		for(int i = 0; i < tree.size(); i++) {
			int current = i + 1;
			struct AdjChar adjChar = {};
			adjChar.nodeChar = tree[i];

			// lavy syn
			if ((2 * current) <= tree.size()) {
				struct Edge edge = {
					counter,
					true,
					tree[current - 1],
					tree[2 * current - 1]
				};
				counter++;

				struct Edge reverseEdge = {
					counter,
					false,
					tree[2 * current - 1],
					tree[current - 1]
				};
				counter++;

				struct AdjencyElement adjency = {
					edge,
					reverseEdge
				};

				adjChar.adjencies.push_back(adjency);
			}

			//pravy syn
			if ((2 * current + 1) <= tree.size()) {
				struct Edge edge = {
					counter,
					true,
					tree[current - 1],
					tree[2 * current ]
				};
				counter++;

				struct Edge reverseEdge = {
					counter,
					false,
					tree[2 * current],
					tree[current - 1]
				};
				counter++;

				struct AdjencyElement adjency = {
					edge,
					reverseEdge
				};
				adjChar.adjencies.push_back(adjency);
			}

			//spetna hrana vracajuca sa na nadradeny uzel
			if ((current / 2 > 0) && ((2 * current) <= tree.size())){
				int index;
				struct AdjencyElement adjency = {};
				int nodesIndex = (i - 1) / 2;
				if (current % 2 == 0) {
					//lavy podstrom
					index = 0;
					adjency.edge = characterNodes[nodesIndex].adjencies[index].reverseEdge;
					adjency.reverseEdge = characterNodes[nodesIndex].adjencies[index].edge;
				} else {
					//pravy podstrom
					index = 1;
					adjency.edge = characterNodes[nodesIndex].adjencies[index].reverseEdge;
					adjency.reverseEdge = characterNodes[nodesIndex].adjencies[index].edge;
				}
				adjency.reverseEdge.isForwardEdge = false;
				adjChar.adjencies.push_back(adjency);				
			}
			characterNodes.push_back(adjChar);
		}
	}

	// Vytvorenie MPI Datatypu struct ProcEdge
    const int nitems=6;
    int          blocklengths[nitems] = {1,1,1,1,1,1};
    MPI_Datatype types[nitems] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_CHAR, MPI_CHAR};
    MPI_Datatype mpi_edge_type;
    MPI_Aint     offsets[nitems];

    offsets[0] = offsetof(struct ProcEdge, myEdgeNumber);
    offsets[1] = offsetof(struct ProcEdge, nextEdgeNumber);
    offsets[2] = offsetof(struct ProcEdge, suffixSumValue);
	offsets[3] = offsetof(struct ProcEdge, isForwardEdge);
    offsets[4] = offsetof(struct ProcEdge, start);
    offsets[5] = offsetof(struct ProcEdge, end);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_edge_type);
    MPI_Type_commit(&mpi_edge_type);

    int initNumber = 1;
    struct ProcEdge procEdge;

    // Rozdistribujeme struktury ProcEdge jednotlivym procesorom podla funkcie ETour
	if (myId == 0) {
		myEdgeNumber = myId + 1;

		procEdge = ETour(characterNodes, myEdgeNumber);
		procEdge.myEdgeNumber = myEdgeNumber;

		for (int i = 0; i < numprocs; i++) {
			struct ProcEdge procEdgeTmp;
			procEdgeTmp = ETour(characterNodes, initNumber);
			procEdgeTmp.myEdgeNumber = initNumber;
			MPI_Send(&procEdgeTmp, 1, mpi_edge_type, i, TAG, MPI_COMM_WORLD);
			initNumber = procEdgeTmp.nextEdgeNumber;
		}
	}
	MPI_Recv(&procEdge, 1, mpi_edge_type, 0, TAG, MPI_COMM_WORLD, &status);

 	//suffix sum
	int value = 0;
	if (numprocs - 1 == myId) {
		value = 0;
	} else {
		value = procEdge.isForwardEdge;
	}
	MPI_Barrier(MPI_COMM_WORLD);

	procEdge.suffixSumValue = 0;
	for (int i = numprocs - 1; i >= 0; i--) {
		int receivedValue = 0;
		if (myId == i) {
			if (myId == numprocs - 1) {
				MPI_Send(&receivedValue, 1, MPI_INT, i - 1, TAG, MPI_COMM_WORLD);
			} else if (i == 0) {
				MPI_Recv(&receivedValue, 1, MPI_INT, i + 1, TAG, MPI_COMM_WORLD, &status);
				procEdge.suffixSumValue = value + receivedValue;
			} else {
				MPI_Recv(&receivedValue, 1, MPI_INT, i + 1, TAG, MPI_COMM_WORLD, &status);
				procEdge.suffixSumValue = value + receivedValue;
				MPI_Send(&procEdge.suffixSumValue, 1, MPI_INT, i - 1, TAG, MPI_COMM_WORLD);
			}	
		}
	} 

	// Kazdy procesor odosle nultemu svoje poradie v suffix sum
	MPI_Send(&procEdge, 1, mpi_edge_type, 0, TAG, MPI_COMM_WORLD);
	if (myId == 0) {
		// Nulty procesor prijme vsetky hodnoty suffix sum od procesorov a ulozi ich do vektoru
		vector<struct ProcEdge> suffixSumResult;
		suffixSumResult.resize(numprocs);
		for (int i = 0; i < numprocs; i++) {
			MPI_Recv(&suffixSumResult[i], 1, mpi_edge_type, i, TAG, MPI_COMM_WORLD, &status);
		}
		endTime = MPI_Wtime();
		// cerr << endTime-startTime;

		//Vypis preorder na zaklade suffix sum
		string output;
		output.resize(tree.size());
		output[0] = procEdge.start;
		printFinalVector(suffixSumResult, &output, tree.size());
		cout << output << endl;
	}

	MPI_Finalize();
    return 0;
}