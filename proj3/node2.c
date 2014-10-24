#include <stdio.h>
#include "project3.h"

extern int TraceLevel;
extern float clocktime;

struct distance_table {
  int costs[MAX_NODES][MAX_NODES];
};
struct distance_table dt2;
struct NeighborCosts   *neighbor2;

/* students to write the following two routines, and maybe some others */

// local vars
struct RoutePacket somePacket;

// prototype functions
void printdt2( int MyNodeNumber, struct NeighborCosts *neighbor, struct distance_table *dtptr );

void rtinit2() {
    int i, j;
    struct distance_table *thisnode2;

    thisnode2 = &dt2;

    // set the temporary packet mincosts to INFINITY
    somePacket.mincost[0] = INFINITY;
    somePacket.mincost[1] = INFINITY;
    somePacket.mincost[2] = INFINITY;
    somePacket.mincost[3] = INFINITY;

    printf("At time t = %f, rtinit2() called\n", clocktime);

    // set links from node i to node j as INFINITY
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            thisnode2->costs[i][j]= INFINITY;
        }
    }

    neighbor2 = getNeighborCosts(2);

    // link weight from node 0
    thisnode2->costs[0][0] = neighbor2->NodeCosts[0];
    thisnode2->costs[1][1] = neighbor2->NodeCosts[1];
    thisnode2->costs[2][2] = neighbor2->NodeCosts[2];
    thisnode2->costs[3][3] = neighbor2->NodeCosts[3];

    // check each distance from node i to node j in comparison to our somePacket
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (somePacket.mincost[i] > thisnode2->costs[i][j]) {
                somePacket.mincost[i] = thisnode2->costs[i][j];
            }
        }
    }

    // set somePacket's sourceID to 2 (we're in node2)
    // use tolayer2 to send to another node, depending on the destID
    somePacket.sourceid = 2;
    somePacket.destid = 0;
    toLayer2(somePacket);
    printf("At time t = %f, node 2 sends packet to node 0 with: %d %d %d %d\n", clocktime, somePacket.mincost[0], somePacket.mincost[1], somePacket.mincost[2], somePacket.mincost[3]);

    somePacket.destid = 1;
    toLayer2(somePacket);
    printf("At time t = %f, node 2 sends packet to node 1 with: %d %d %d %d\n", clocktime, somePacket.mincost[0], somePacket.mincost[1], somePacket.mincost[2], somePacket.mincost[3]);

    somePacket.destid = 3;
    toLayer2(somePacket);
    printf("At time t = %f, node 2 sends packet to node 3 with: %d %d %d %d\n", clocktime, somePacket.mincost[0], somePacket.mincost[1], somePacket.mincost[2], somePacket.mincost[3]);

    printdt2(2, neighbor2, thisnode2);
    printf("We are done with rtinit2 routine\n");
}


void rtupdate2( struct RoutePacket *rcvdpkt ) {
    int i, j, rcvdSource, update = 0;
    struct distance_table *thisnode2;

    thisnode2 = &dt2;

    for (i = 0; i < 4; i++) {
            somePacket.mincost[i] = INFINITY;
    }

    printf("Now we are in rtupdate2()\n");

    // check what sourceID of received packet is
    rcvdSource = rcvdpkt->sourceid;
    printf("Received packet's source ID is %d\n", rcvdSource);

    for (i = 0; i < 4; i++) {
        // go through and see if we need to update distance-table with new mincost
        if (thisnode2->costs[i][rcvdSource] > (thisnode2->costs[rcvdSource][rcvdSource] + rcvdpkt->mincost[i])) {
            thisnode2->costs[i][rcvdSource] = (thisnode2->costs[rcvdSource][rcvdSource] + rcvdpkt->mincost[i]);
            update = 1;
        }
    }

    // for this case, we update distance table
    if (update == 1) {
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                if (somePacket.mincost[i] > thisnode2->costs[i][j]) {
                    somePacket.mincost[i] = thisnode2->costs[i][j];
                }
            }
        }
        printf("We updated distance table from rtupdate2()\n");

        // set somePacket's sourceID to 2 (we're in node2)
        // use tolayer2 to send to another node, depending on the destID
        somePacket.sourceid = 2;
        somePacket.destid = 0;
        toLayer2(somePacket);
        printf("At time t = %f, node 2 sends packet to node 0 with: %d %d %d %d\n", clocktime, somePacket.mincost[0], somePacket.mincost[1], somePacket.mincost[2], somePacket.mincost[3]);

        somePacket.destid = 1;
        toLayer2(somePacket);
        printf("At time t = %f, node 2 sends packet to node 1 with: %d %d %d %d\n", clocktime, somePacket.mincost[0], somePacket.mincost[1], somePacket.mincost[2], somePacket.mincost[3]);

        somePacket.destid = 3;
        toLayer2(somePacket);
        printf("At time t = %f, node 2 sends packet to node 3 with: %d %d %d %d\n", clocktime, somePacket.mincost[0], somePacket.mincost[1], somePacket.mincost[2], somePacket.mincost[3]);
    }

    printdt2(2, neighbor2, thisnode2);

    printf("We are done with rtupdate2()\n");
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
void printdt2( int MyNodeNumber, struct NeighborCosts *neighbor, 
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
}    // End of printdt2

