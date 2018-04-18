#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm> 
#include <math.h>
#include <typeinfo>

using namespace std;

#define TAG 0

struct Edge {
	int number;
	bool isForwardEdge;
	char start;
	char end;
};

struct AdjencyElement {
	struct Edge edge;
	struct Edge reverseEdge;
};

struct AdjChar {
	char nodeChar;
	vector<struct AdjencyElement> adjencies;
};

struct ProcEdge {
	int myEdgeNumber;
	int nextEdgeNumber;
	bool isForwardEdge;
	char start;
	char end;
};

void printStructEdge(struct Edge edge) {
	cout << "Number:" << edge.number << endl;
	cout << "Is forward edge " << edge.isForwardEdge << endl;
	cout << "Startovaci symbol: " << edge.start << endl;
	cout << "Konecny symbol: " << edge.end << endl;
}

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

void printVectorCharacterNodes(vector<struct AdjChar> characterNodes) {
	for(int i = 0; i < characterNodes.size(); i++) {
		cout << '\n';
		cout << "Uzol symbol: " << characterNodes[i].nodeChar << endl;
		printVectorOfAdjencies(characterNodes[i].adjencies);
	}
}

void printVector(vector<int> vec) {
	for(int i = 0; i < vec.size(); i++)
	{
		if(vec[i] == -1) break;
		cout << vec[i] << " ";
	}
	cout << endl;
}

struct ProcEdge ETour(vector<struct AdjChar> characterNodes, int myEdgeNumber) {
	struct ProcEdge procEdge = {
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
			if (element.edge.number == myEdgeNumber && procEdge.nextEdgeNumber == -1) {
				procEdge.nextEdgeNumber = element.reverseEdge.number;
				procEdge.isForwardEdge = element.edge.isForwardEdge;
				procEdge.start = element.edge.start;
				procEdge.end = element.edge.end;
			}
			if (element.reverseEdge.number == myEdgeNumber) {
				if ((characterNodes[i].adjencies.size() - 1) == j) {
					procEdge.nextEdgeNumber = characterNodes[i].adjencies[0].edge.number;
					procEdge.start = element.reverseEdge.start;
					procEdge.end = element.reverseEdge.end;
				} else {
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

	// cout << tree << endl;
	int counter = 1;

	for(int i = 0; i < tree.size(); i++) {
		int current = i + 1;
		struct AdjChar adjChar = {};
		adjChar.nodeChar = tree[i];

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

		if ((current / 2 > 0) && ((2 * current) <= tree.size())){
			int index;
			struct AdjencyElement adjency = {};
			int nodesIndex = (i - 1) / 2;
			if (current % 2 == 0) {
				index = 0;
				adjency.edge = characterNodes[nodesIndex].adjencies[index].reverseEdge;
				adjency.reverseEdge = characterNodes[nodesIndex].adjencies[index].edge;
			} else {
				index = 1;
				adjency.edge = characterNodes[nodesIndex].adjencies[index].reverseEdge;
				adjency.reverseEdge = characterNodes[nodesIndex].adjencies[index].edge;
			}
			adjency.reverseEdge.isForwardEdge = false;
			adjChar.adjencies.push_back(adjency);				
		}
		characterNodes.push_back(adjChar);
		// cout << typeid(tree[i]).name() << endl;
	}

    const int nitems=5;
    int          blocklengths[5] = {1,1,1,1,1};
    MPI_Datatype types[5] = {MPI_INT, MPI_INT, MPI_INT, MPI_CHAR, MPI_CHAR};
    MPI_Datatype mpi_edge_type;
    MPI_Aint     offsets[5];

    offsets[0] = offsetof(struct ProcEdge, myEdgeNumber);
    offsets[1] = offsetof(struct ProcEdge, nextEdgeNumber);
	offsets[2] = offsetof(struct ProcEdge, isForwardEdge);
    offsets[3] = offsetof(struct ProcEdge, start);
    offsets[4] = offsetof(struct ProcEdge, end);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_edge_type);
    MPI_Type_commit(&mpi_edge_type);

    int initNumber = 1;
    struct ProcEdge procEdge;
	if (myId == 0) {
	    // cout << "numprocs " << numprocs << endl;
		// printVectorCharacterNodes(characterNodes);
		myEdgeNumber = myId + 1;

		procEdge = ETour(characterNodes, myEdgeNumber);
		procEdge.myEdgeNumber = myEdgeNumber;

		for (int i = 0; i < numprocs; i++) {
			struct ProcEdge procEdgeTmp;
			procEdgeTmp = ETour(characterNodes, initNumber);
			procEdgeTmp.myEdgeNumber = initNumber;
			// cout << "My edge number: " << procEdgeTmp.myEdgeNumber << endl << "ETour next edge number: " << procEdgeTmp.nextEdgeNumber << endl << " my Id: " << myId + 1 << endl << endl;
			MPI_Send(&procEdgeTmp, 1, mpi_edge_type, i, TAG, MPI_COMM_WORLD);
			initNumber = procEdgeTmp.nextEdgeNumber;
		}
	}
	MPI_Recv(&procEdge, 1, mpi_edge_type, 0, TAG, MPI_COMM_WORLD, &status);
	// cout << myId << endl;
 	MPI_Barrier(MPI_COMM_WORLD);
	// cout << "My edge number: " << procEdge.myEdgeNumber << endl << "ETour next edge number: " << procEdge.nextEdgeNumber << endl << " my Id: " << myId + 1 << endl << endl;

	int initialValue = 1;
	if (myId == 0) {
		MPI_Send(&initialValue, 1, MPI_INT, procEdge.nextEdgeNumber - 1, TAG, MPI_COMM_WORLD);
	} else {
		MPI_Recv(&initialValue, 1, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &status);
		initialValue++;
		if (procEdge.nextEdgeNumber != 1) {
			MPI_Send(&initialValue, 1, MPI_INT, procEdge.nextEdgeNumber - 1, TAG, MPI_COMM_WORLD);
		}
	}

	vector<int> suffixSum;
	int value = 0;
	suffixSum.reserve(numprocs);

	if (numprocs - 1 == myId) {
		// cout << "My edge number: " << procEdge.myEdgeNumber << endl << "ETour next edge number: " << procEdge.nextEdgeNumber << endl << "Poradie: " << initialValue << endl << endl;
		value = 0;
	} else {
		value = procEdge.isForwardEdge;
	}
	MPI_Barrier(MPI_COMM_WORLD);

	// cout << "cpu: " << myId + 1 << " initial value " << initialValue << endl;
	int newValue = 0;
	for (int i = numprocs - 1; i >= 0; i--) {
		int receivedValue = 0;

		if (myId == i) {
			// cout << i << " id proc " << myId + 1 << endl;
			if (myId == numprocs - 1) {
				MPI_Send(&receivedValue, 1, MPI_INT, i - 1, TAG, MPI_COMM_WORLD);
				// cout << receivedValue << endl;
			} else if (i == 0) {
				MPI_Recv(&receivedValue, 1, MPI_INT, i + 1, TAG, MPI_COMM_WORLD, &status);
				newValue = value + receivedValue;
				// cout << newValue << endl;	
			} else {
				// cout << "cakam" << endl;
				MPI_Recv(&receivedValue, 1, MPI_INT, i + 1, TAG, MPI_COMM_WORLD, &status);
				newValue = value + receivedValue;
				// cout << "New valu " << newValue << " old " << value << " received " << receivedValue << " id proc " << myId + 1 << " id hrany " << procEdge.myEdgeNumber << " initiialValue " << initialValue << endl;
				MPI_Send(&newValue, 1, MPI_INT, i - 1, TAG, MPI_COMM_WORLD);
			}	
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if (procEdge.isForwardEdge) {
		cout << "New valu " << newValue << "hrana c" << procEdge.myEdgeNumber << endl;
	}

	MPI_Finalize();
    return 0;
}