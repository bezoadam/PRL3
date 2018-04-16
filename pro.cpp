#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm> 
#include <math.h>
#include <typeinfo>

using namespace std;

struct Edge {
	char start;
	char end;
};

struct AdjencyElement {
	struct Edge edge;
	struct Edge reverseEdge;
};

void printStructEdge(struct Edge edge) {
	cout << "Startovaci symbol: " << edge.start << endl;
	cout << "Konecny symbol: " << edge.end << endl;
}

void printVectorOfAdjencies(vector<struct AdjencyElement> adjencies) {
	for(int i = 0; i < adjencies.size(); i++) {
		cout << "Uzol c. " << i << endl;
		cout << "Edge:" << endl;
		printStructEdge(adjencies[i].edge);
		cout << "Edge reverse:" << endl;
		printStructEdge(adjencies[i].reverseEdge);
	}
}

int main(int argc, char* argv[])
{
   	int numprocs, myId;
	MPI_Status status;	//struct- obsahuje kod- source, tag, error
	string tree(argv[1]);
	vector<struct AdjencyElement> adjencies;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);       // zistime pocet procesorov
	MPI_Comm_rank(MPI_COMM_WORLD, &myId);           // zistime id svojho procesoru

	if (myId == 0) {
		cout << tree << endl;

		for(int i = 0; i < tree.size(); i++) {
			int current = i + 1;
			if ((2 * current) <= tree.size()) {
				struct Edge edge = {
					tree[current - 1],
					tree[2 * current - 1]
				};

				struct Edge reverseEdge = {
					tree[2 * current - 1],
					tree[current - 1]
				};

				struct AdjencyElement adjency = {
					edge,
					reverseEdge
				};

				adjencies.push_back(adjency);
			}

			if ((2 * current + 1) <= tree.size()) {
				struct Edge edge = {
					tree[current - 1],
					tree[2 * current ]
				};

				struct Edge reverseEdge = {
					tree[2 * current],
					tree[current - 1]
				};

				struct AdjencyElement adjency = {
					edge,
					reverseEdge
				};
				adjencies.push_back(adjency);
			}	
			// cout << typeid(tree[i]).name() << endl;
		}

		printVectorOfAdjencies(adjencies);
	}

	MPI_Finalize();
    return 0;
}