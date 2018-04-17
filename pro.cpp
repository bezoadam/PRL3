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
		false
	};
	for(int i = 0; i < characterNodes.size(); i++) {
		int leftSideIndex = 0;
		for(int j = 0; j < characterNodes[i].adjencies.size(); j++) {
			struct AdjencyElement element = characterNodes[i].adjencies[j];
			if (element.edge.number == myEdgeNumber && procEdge.nextEdgeNumber == -1) {
				procEdge.nextEdgeNumber = element.reverseEdge.number;
				procEdge.isForwardEdge = element.edge.isForwardEdge;
			}
			if (element.reverseEdge.number == myEdgeNumber) {
				if ((characterNodes[i].adjencies.size() - 1) == j) {
					procEdge.nextEdgeNumber = characterNodes[i].adjencies[0].edge.number;
				} else {
					procEdge.nextEdgeNumber = characterNodes[i].adjencies[j + 1].edge.number;
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

	if (myId == 0) {
		printVectorCharacterNodes(characterNodes);
	}
	myEdgeNumber = myId + 1;

	struct ProcEdge procEdge = ETour(characterNodes, myEdgeNumber);
	procEdge.myEdgeNumber = myEdgeNumber;


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
		cout << "My edge number: " << procEdge.myEdgeNumber << endl << "ETour next edge number: " << procEdge.nextEdgeNumber << endl << "Poradie: " << initialValue << endl << endl;
		value = 0;
	} else {
		value = procEdge.isForwardEdge;
	}
	MPI_Barrier(MPI_COMM_WORLD);


	int newValue = 0;
	for (int i = numprocs - 1; i >= 0; i--) {
		int receivedValue = 0;
		if (myId == i) {
			if (myId == numprocs - 1) {
				MPI_Send(&value, 1, MPI_INT, i - 1, TAG, MPI_COMM_WORLD);
			} else if (i == 0) {
				MPI_Recv(&receivedValue, 1, MPI_INT, i + 1, TAG, MPI_COMM_WORLD, &status);
				newValue = value + receivedValue;			
			} else {
				MPI_Recv(&receivedValue, 1, MPI_INT, i + 1, TAG, MPI_COMM_WORLD, &status);
				newValue = value + receivedValue;
				MPI_Send(&newValue, 1, MPI_INT, i - 1, TAG, MPI_COMM_WORLD);
			}	
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if (procEdge.isForwardEdge) {
		cout << "New valu " << newValue << endl;
	}

	MPI_Finalize();
    return 0;
}