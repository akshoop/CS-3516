#include <stdio.h>
#include "project3.h"

extern int TraceLevel;
extern float clocktime;

struct distance_table {
  int costs[MAX_NODES][MAX_NODES];
};
struct distance_table dt1;
struct NeighborCosts   *neighbor1;

/* students to write the following two routines, and maybe some others */

// local vars
struct RoutePacket somePacket;

// prototype functions
void printdt1( int MyNodeNumber, struct NeighborCosts *neighbor, struct distance_table *dtptr );

void rtinit1() {
    int i, j;
    struct distance_table *thisnode1;

    thisnode1 = &dt1;

    // set the temporary packet mincosts to INFINITY
    somePacket.mincost[0] = INFINITY;
    somePacket.mincost[1] = INFINITY;
    somePacket.mincost[2] = INFINITY;
    somePacket.mincost[3] = INFINITY;

    printf("At time t = %f, rtinit1() called\n", clocktime);

    // set links from node i to node j as INFINITY
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            thisnode1->costs[i][j]= INFINITY;
        }
    }

    neighbor1 = getNeighborCosts(1);

    // link weight from node 0
    thisnode1->costs[0][0] = neighbor1->NodeCosts[0];
    thisnode1->costs[1][1] = neighbor1->NodeCosts[1];
    thisnode1->costs[2][2] = neighbor1->NodeCosts[2];
    thisnode1->costs[3][3] = neighbor1->NodeCosts[3];

    // check each distance from node i to node j in comparison to our somePacket
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (somePacket.mincost[i] > thisnode1->costs[i][j]) {
                somePacket.mincost[i] = thisnode1->costs[i][j];
            }
        }
    }

    // set somePacket's sourceID to 1 (we're in node1)
    // use tolayer2 to send to another node, depending on the destID
    somePacket.sourceid = 1;
    somePacket.destid = 0;
    toLayer2(somePacket);
    printf("At time t = %f, node 1 sends packet to node 0 with: %d %d %d %d\n", clocktime, somePacket.mincost[0], somePacket.mincost[1], somePacket.mincost[2], somePacket.mincost[3]);

    somePacket.destid = 2;
    toLayer2(somePacket);
    printf("At time t = %f, node 1 sends packet to node 2 with: %d %d %d %d\n", clocktime, somePacket.mincost[0], somePacket.mincost[1], somePacket.mincost[2], somePacket.mincost[3]);

    somePacket.destid = 3;
    toLayer2(somePacket);
    printf("At time t = %f, node 1 sends packet to node 3 with: %d %d %d %d\n", clocktime, somePacket.mincost[0], somePacket.mincost[1], somePacket.mincost[2], somePacket.mincost[3]);

    printdt1(1, neighbor1, thisnode1);
    printf("We are done with rtinit1 routine\n");
}


void rtupdate1( struct RoutePacket *rcvdpkt ) {
    int i, j, rcvdSource, update = 0;
    struct distance_table *thisnode1;

    thisnode1 = &dt1;

    for (i = 0; i < 4; i++) {
            somePacket.mincost[i] = INFINITY;
    }

    printf("Now we are in rtupdate1()\n");

    // check what sourceID of received packet is
    rcvdSource = rcvdpkt->sourceid;
    printf("Received packet's source ID is %d\n", rcvdSource);

    for (i = 0; i < 4; i++) {
        // go through and see if we need to update distance-table with new mincost
        if (thisnode1->costs[i][rcvdSource] > (thisnode1->costs[rcvdSource][rcvdSource] + rcvdpkt->mincost[i])) {
            thisnode1->costs[i][rcvdSource] = (thisnode1->costs[rcvdSource][rcvdSource] + rcvdpkt->mincost[i]);
            update = 1;
        }
    }

    // for this case, we update distance table
    if (update == 1) {
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (somePacket.mincost[i] > thisnode1->costs[i][j]) {
                    somePacket.mincost[i] = thisnode1->costs[i][j];
                }
            }
        }
        printf("We updated distance table from rtupdate1()\n");

        // set somePacket's sourceID to 1 (we're in node1)
        // use tolayer2 to send to another node, depending on the destID
        somePacket.sourceid = 1;
        somePacket.destid = 0;
        toLayer2(somePacket);
        printf("At time t = %f, node 1 sends packet to node 0 with: %d %d %d %d\n", clocktime, somePacket.mincost[0], somePacket.mincost[1], somePacket.mincost[2], somePacket.mincost[3]);

        somePacket.destid = 2;
        toLayer2(somePacket);
        printf("At time t = %f, node 1 sends packet to node 2 with: %d %d %d %d\n", clocktime, somePacket.mincost[0], somePacket.mincost[1], somePacket.mincost[2], somePacket.mincost[3]);

        somePacket.destid = 3;
        toLayer2(somePacket);
        printf("At time t = %f, node 1 sends packet to node 3 with: %d %d %d %d\n", clocktime, somePacket.mincost[0], somePacket.mincost[1], somePacket.mincost[2], somePacket.mincost[3]);
    }

    printdt1(1, neighbor1, thisnode1);

    printf("We are done with rtupdate1()\n");
}


/////////////////////////////////////////////////////////////////////
//  printdt
//  This routine is being supplied to you.  It is the same code in
//  each node and is tailored based on the input arguments.
//  Required arguments:
//  MyNodeNumber:  This routine assumes that you know your node
//                 number and supply it when making this call.
//  struct NeighborCosts *neighbor:  A pointer to the structure 
//                 that's supplied via a call to getNeighborCosts().
//                 It tells this print routine the configuration
//                 of nodes surrounding the node we're working on.
//  struct distance_table *dtptr: This is the running record of the
//                 current costs as seen by this node.  It is 
//                 constantly updated as the node gets new
//                 messages from other nodes.
/////////////////////////////////////////////////////////////////////
void printdt1( int MyNodeNumber, struct NeighborCosts *neighbor, 
		struct distance_table *dtptr ) {
    int       i, j;
    int       TotalNodes = neighbor->NodesInNetwork;     // Total nodes in network
    int       NumberOfNeighbors = 0;                     // How many neighbors
    int       Neighbors[MAX_NODES];                      // Who are the neighbors

    // Determine our neighbors 
    for ( i = 0; i < TotalNodes; i++ )  {
        if (( neighbor->NodeCosts[i] != INFINITY ) && i != MyNodeNumber )  {
            Neighbors[NumberOfNeighbors] = i;
            NumberOfNeighbors++;
        }
    }
    // Print the header
    printf("                via     \n");
    printf("   D%d |", MyNodeNumber );
    for ( i = 0; i < NumberOfNeighbors; i++ )
        printf("     %d", Neighbors[i]);
    printf("\n");
    printf("  ----|-------------------------------\n");

    // For each node, print the cost by travelling thru each of our neighbors
    for ( i = 0; i < TotalNodes; i++ )   {
        if ( i != MyNodeNumber )  {
            printf("dest %d|", i );
            for ( j = 0; j < NumberOfNeighbors; j++ )  {
                    printf( "  %4d", dtptr->costs[i][Neighbors[j]] );
            }
            printf("\n");
        }
    }
    printf("\n");
}    // End of printdt1

