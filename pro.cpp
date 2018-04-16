#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm> 
#include <math.h>
#include <typeinfo>

using namespace std;

struct Edge {
	int number;
	char start;
	char end;
};

struct AdjencyElement {
	int nextEdgeNumber;
	struct Edge edge;
	struct Edge reverseEdge;
};

struct AdjChar {
	char nodeChar;
	vector<struct AdjencyElement> adjencies;
};

void printStructEdge(struct Edge edge) {
	cout << "Number:" << edge.number << endl;
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
		cout << "Next edge: " << adjencies[i].nextEdgeNumber << endl;
	}
}

void printVectorCharacterNodes(vector<struct AdjChar> characterNodes) {
	for(int i = 0; i < characterNodes.size(); i++) {
		cout << '\n';
		cout << "Uzol symbol: " << characterNodes[i].nodeChar << endl;
		printVectorOfAdjencies(characterNodes[i].adjencies);
	}
}


int main(int argc, char* argv[])
{
   	int numprocs, myId;
	MPI_Status status;	//struct- obsahuje kod- source, tag, error
	string tree(argv[1]);
	vector<struct AdjChar> characterNodes;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // zistime pocet procesorov
	MPI_Comm_rank(MPI_COMM_WORLD, &myId);           // zistime id svojho procesoru

	if (myId == 0) {
		cout << tree << endl;
		int counter = 1;

		for(int i = 0; i < tree.size(); i++) {
			int current = i + 1;
			struct AdjChar adjChar = {};
			adjChar.nodeChar = tree[i];

			if ((2 * current) <= tree.size()) {
				struct Edge edge = {
					counter,
					tree[current - 1],
					tree[2 * current - 1]
				};
				counter++;

				struct Edge reverseEdge = {
					counter,
					tree[2 * current - 1],
					tree[current - 1]
				};
				counter++;

				struct AdjencyElement adjency = {
					counter,
					edge,
					reverseEdge
				};

				adjChar.adjencies.push_back(adjency);
			}

			if ((2 * current + 1) <= tree.size()) {
				struct Edge edge = {
					counter,
					tree[current - 1],
					tree[2 * current ]
				};
				counter++;

				struct Edge reverseEdge = {
					counter,
					tree[2 * current],
					tree[current - 1]
				};
				counter++;

				struct AdjencyElement adjency = {
					counter,
					edge,
					reverseEdge
				};
				adjChar.adjencies.push_back(adjency);
			}

			if ((current / 2 > 0) && ((2 * current) <= tree.size())){
				int divide = current / 2;
				// struct Edge edge = {
				// 	tree[current - 1],
				// 	tree[divide - 1]
				// };

				// struct Edge reverseEdge = {
				// 	tree[divide - 1],
				// 	tree[current - 1]
				// };

				// struct AdjencyElement adjency = {
				// 	edge,
				// 	reverseEdge
				// };

				int index;
				struct AdjencyElement adjency = {};
				int nodesIndex = (i - 1) / 2;
				if (current % 2 == 0) {
					index = 0;
					adjency.nextEdgeNumber = characterNodes[nodesIndex].adjencies[1].edge.number;
					adjency.edge = characterNodes[nodesIndex].adjencies[index].reverseEdge,
					adjency.reverseEdge = characterNodes[nodesIndex].adjencies[index].edge;
				} else {
					index = 1;
					adjency.nextEdgeNumber = 0;
					adjency.edge = characterNodes[nodesIndex].adjencies[index].reverseEdge,
					adjency.reverseEdge = characterNodes[nodesIndex].adjencies[index].edge;
				}
				adjChar.adjencies.push_back(adjency);				
			}
			characterNodes.push_back(adjChar);
			// cout << typeid(tree[i]).name() << endl;
		}
		printVectorCharacterNodes(characterNodes);
	}

	MPI_Finalize();
    return 0;
}