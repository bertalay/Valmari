#include <iostream>
#include <algorithm>

/* Refinable partition */
int *numMarked, *markedSets, numSetsMarked = 0;

// temporary worksets
struct partition{
    int numSets;
    int *elements, *elemInv, *setMap, *setStartIndices, *setEndIndices;

    //Initializes the partition. Takes in the number of elements in the partition
    void init(int size) {
        numSets = bool(size);
        elements = new int[size];
        elemInv = new int[size]; //elemInv[elements[i]] = i
        setMap = new int[size];
        setStartIndices = new int[size];
        setEndIndices = new int[size];
        for(int i = 0; i < size; ++i) {
            elements[i] = elemInv[i] = i;
            setMap[i] = 0;
        }
        if(numSets) {
            setStartIndices[0] = 0;
            setEndIndices[0] = size;
        }
    }

    //Marks an element of the partition
    void mark(int elem) {
        int set = setMap[elem];
        int locationOf = elemInv[elem];
        int indexToPlace = setStartIndices[set] + numMarked[set];

        //Swaps marked element to the front of the set
        elements[locationOf] = elements[indexToPlace];
        elemInv[elements[locationOf]] = locationOf;
        elements[indexToPlace] = elem;
        elemInv[elem] = indexToPlace;

        numMarked[set]++;
        if(numMarked[set] == 1) { //First marked element of set
            numSetsMarked++;
            markedSets[numSetsMarked] = set;
        }
    }

    //For each marked block of the partition, splits it if it is not completely marked
    void split() {
        while(numSetsMarked) {  //Until no marked sets remain
            int set = markedSets[--numSetsMarked];
            if(numMarked[set] == 0) {
                printf("fuck");
            }
            int unmarkedStart = setStartIndices[set] + numMarked[set];
            if(unmarkedStart == setEndIndices[set]) {   //All elements of the block are marked
                numMarked[set] = 0;
                continue;
            }

            //The smaller split set gets the new label
            if(numMarked[set] <= setEndIndices[set] - unmarkedStart) {
                setStartIndices[numSets] = setStartIndices[set];
                setEndIndices[numSets] = setStartIndices[set] = unmarkedStart;
            }
            else {
                setEndIndices[numSets] = setEndIndices[set];
                setStartIndices[numSets] = setEndIndices[set] = unmarkedStart;
            }

            //Relabeling the sets
            for( int i = setStartIndices[numSets]; i < setEndIndices[numSets]; ++i ) {
                setMap[elements[i]] = numSets;
            }
            numMarked[set] = numMarked[numSets++] = 0;
        }
    }
};

partition blocks;     // blocks (consist of states)
partition transitions;     // cords (consist of transitions)
int numStates;    // number of states
int numTransitions;    // number of transitions
int numFinal;    // number of final states
int q0;    // initial state

//Description of transitions
int *T;    // tails of transitions
int *L;    // labels of transitions
int *H;    // heads of transition

//Partial order by label
bool cmp( int i, int j ) {
    return L[i] < L[j];
}

/* Adjacent transitions */
int *A, *F;
void make_adjacent(int K[]) {
    int q, t;
    for(q = 0; q <= numStates; ++q) {
        F[q] = 0;
    }
    for(t = 0; t < numTransitions; ++t) {
        ++F[K[t]];
    }
    for(q = 0; q < numStates; ++q)
        F[q+1] += F[q];
    for(t = numTransitions; t--; ) {
        A[--F[K[t]]] = t;
    }
}


/* Removal of irrelevant parts */
int numReached = 0;   // number of reached states
inline void reach(int q) {
    int indexOf = blocks.elemInv[q];
    if(indexOf >= numReached) {
        //Swap q into the front, reached portion of the partition
        blocks.elements[indexOf] = blocks.elements[numReached];
        blocks.elemInv[blocks.elements[indexOf]] = indexOf;
        blocks.elements[numReached] = q;
        blocks.elemInv[q] = numReached;
        numReached++;
    }
}

void rem_unreachable(int T[], int H[]) {
    make_adjacent(T);
    int i, j;
    for(i = 0; i < numReached; ++i) {   //For each reachable element
        for(j = F[blocks.elements[i]]; j < F[blocks.elements[i] + 1]; ++j) {    //For the neighbors of the element
            reach(H[A[j]]);
        }
    }
    j=0;

    //Put all the transitions from reached nodes to the front of the transition arrays
    for(int t = 0; t < numTransitions; ++t) {
        if(blocks.elemInv[T[t]] < numReached) {
            H[j] = H[t];
            L[j] = L[t];
            T[j] = T[t];
            ++j;
        }
    }
    numTransitions = j;
    blocks.setEndIndices[0] = numReached;
    numReached = 0;
}


/* Main program */
int main() {
    /* Read sizes and reserve most memory */
    std::cin >> numStates >> numTransitions >> q0 >> numFinal;
    T = new int[numTransitions];
    L = new int[numTransitions];
    H = new int[numTransitions];
    blocks.init(numStates);
    A = new int[numTransitions];
    F = new int[numStates+1];
    /* Read transitions */
    for( int t = 0; t < numTransitions; ++t ) {
        std::cin >> T[t] >> L[t] >> H[t];
    }
    //Remove states not reachable from initial
    reach(q0);
    rem_unreachable( T, H );

    //Remove states which cannot reach a final state
    for(int i = 0; i < numFinal; ++i) {
        int q;
        std::cin >> q;
        if( blocks.elemInv[q] < blocks.setEndIndices[0] ) {
            reach(q);
        }
    }
    numFinal = numReached;
    rem_unreachable( H, T );

    /* Make initial partition */
    markedSets = new int[numTransitions+1];
    numMarked = new int[numTransitions+1];
    numMarked[0] = numFinal;
    if(numFinal) {
        markedSets[numSetsMarked++] = 0;
        blocks.split();
    }

    /* Make transition partition */
    transitions.init(numTransitions);
    if(numTransitions) {
        std::sort(transitions.elements, transitions.elements + numTransitions, cmp);
        transitions.numSets = numMarked[0] = 0;
        int a = L[transitions.elements[0]];
        for(int i = 0; i < numTransitions; ++i) {
            int t = transitions.elements[i];
            if( L[t] != a ) {
                a = L[t];
                transitions.setEndIndices[transitions.numSets++] = i;
                transitions.setStartIndices[transitions.numSets] = i;
                numMarked[transitions.numSets] = 0;
            }
            transitions.setMap[t] = transitions.numSets;
            transitions.elemInv[t] = i;
        }
    }
    transitions.setEndIndices[transitions.numSets++] = numTransitions;

    /* Split blocks and cords */
    make_adjacent(H);
    int b = 1, c = 0, i, j;
    while(c < transitions.numSets) {
        printf("%d should be 0", numSetsMarked);
        for(i = transitions.setStartIndices[c]; i < transitions.setEndIndices[c]; ++i) {
            blocks.mark(T[transitions.elements[i]]);
        }
        blocks.split();
        ++c;
        while(b < blocks.numSets) {
            for( i = blocks.setStartIndices[b]; i < blocks.setEndIndices[b]; ++i ) {
                for(j = F[blocks.elements[i]];j < F[blocks.elements[i]+1]; ++j) {
                    transitions.mark(A[j]);
                }
            }
            transitions.split();
            ++b;
        }
    }

    /* Count the numbers of transitions and final states in the result */
    int mo = 0, fo = 0;
    for(int t = 0; t < numTransitions; ++t) {
        if(blocks.elemInv[T[t]] == blocks.setStartIndices[blocks.setMap[T[t]]]) {
            ++mo;
        }
    }
    for(int b = 0; b < blocks.numSets; ++b) {
        if(blocks.setStartIndices[b] < numFinal) {
            ++fo;
        }
    }
    /* Print the result */
    std::cout << "Number of states: " << blocks.numSets
              << "\nNumber of transitions: " << mo
              << "\nInitial State: " << blocks.setMap[q0]
              << "\nNumber of final states: " << fo
              << "\nTransitions\n";
    for( int t = 0; t < numTransitions; ++t ) {
        if( blocks.elemInv[T[t]] == blocks.setStartIndices[blocks.setMap[T[t]]] ) {
            std::cout << blocks.setMap[T[t]] << ' ' << L[t]<< ' ' << blocks.setMap[H[t]] << '\n';
        }
    }
    std::cout << "Final States\n";
    for( int b = 0; b < blocks.numSets; ++b ) {
        if(blocks.setStartIndices[b] < numFinal) {
            std::cout << b << '\n';
        }
    }
}