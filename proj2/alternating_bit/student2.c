#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "project2.h"
 
/* ***************************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for Project 2, unidirectional or bidirectional
   data transfer protocols from A to B and B to A.
   Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets may be delivered out of order.

   Compile as gcc -g project2.c student2.c -o p2
**********************************************************************/

#define TIMERVAL 20
#define A 0
#define B 1
#define WIDTH (8 * sizeof(uint32_t))
#define TOPBIT (1 << (WIDTH - 1))
#define POLYNOMIAL 0x04C11DB7

// prototype functions
int complement(int);
uint32_t create_checksum(struct pkt);
int check_checksum(struct pkt);
uint32_t crc32(uint32_t, const void *, size_t);
uint32_t calculateChksm(uint32_t, uint32_t, char*, int);
int checkChksm(uint32_t, uint32_t, char*, int, int);

// int i for loop
int i;
// check if receiver got proper packet
int gotpacket;
// first ACK value will be 0
int ACK = 0;
// first SEQ value will be 0
int SEQ = 0;
// state of A (0 means able to send data, 1 means waiting for ACK)
int state_of_A = 1;
// state of B (0 means waiting for packet, 1 means able to send ACK)
int state_of_B = 0;
// keep a buffed packet for A incase a timeout occurs with a delivered packet
struct pkt buffedPacketA;
// keep a buffed packet for B incase a timeout occurs with a delivered ACK
struct pkt buffedPacketB;

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/* 
 * The routines you will write are detailed below. As noted above, 
 * such procedures in real-life would be part of the operating system, 
 * and would be called by other procedures in the operating system.  
 * All these routines are in layer 4.
 */

/* 
 * A_output(message), where message is a structure of type msg, containing 
 * data to be sent to the B-side. This routine will be called whenever the 
 * upper layer at the sending side (A) has a message to send. It is the job 
 * of your protocol to insure that the data in such a message is delivered 
 * in-order, and correctly, to the receiving side upper layer.
 */
void A_output(struct msg message) {
	message.data[20] = 0;
	// check if waiting for ACK
	if (state_of_A == 1) {
		printf("\n----A is currently waiting for ACK value %d----\n", ACK);
	}
	else {
		struct pkt thisPacket;

		for (i = 0; i < 20; i++) {
			thisPacket.payload[i] = message.data[i];
		}
		thisPacket.seqnum = SEQ;
		thisPacket.acknum = complement(ACK);
		thisPacket.checksum = (int)calculateChksm(thisPacket.seqnum, thisPacket.acknum, thisPacket.payload, sizeof(thisPacket.payload));

		state_of_A = complement(state_of_A);
		SEQ = complement(SEQ);

		// keep a buffer of thisPacket in case a timeout occurs
		buffedPacketA = thisPacket;
		strcpy(buffedPacketA.payload, thisPacket.payload);

		printf("\n----A is currently sending packet.----\n");
		printf("Sequence number: %d\n", thisPacket.seqnum);
		printf("Checksum: %d\n", thisPacket.checksum);
		printf("Payload: %s\n", thisPacket.payload);
		printf("A is expecting ACK value of %d\n", ACK);

		// send packet from upper layer into the network
		tolayer3(A, thisPacket);
		// start A's timer, with time of 20 until interrupt happens
		startTimer(A, TIMERVAL);
	}
}

/*
 * Just like A_output, but residing on the B side.  USED only when the 
 * implementation is bi-directional.
 */
void B_output(struct msg message)  {
	// NOT USED FOR THIS PROJECT
}

/* 
 * A_input(packet), where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the B-side (i.e., as a result 
 * of a tolayer3() being done by a B-side procedure) arrives at the A-side. 
 * packet is the (possibly corrupted) packet sent from the B-side.
 */
void A_input(struct pkt packet) {

	// check if the received packet's acknum is the ACK value we're looking for
	// also check if the checksum is correct
	if (packet.acknum == ACK && checkChksm(packet.seqnum, packet.acknum, packet.payload, sizeof(packet.payload), packet.checksum)) {
		// stop A's timer
		stopTimer(A);

		printf("\n----A has received the proper packet, with ACK value: %d----\n", packet.acknum);

		ACK = complement(ACK);
		state_of_A = complement(state_of_A);
	}

	else {
		printf("\n----A got incorrect packet.----\n");
		printf("A was expecting ACK value %d but packet acknum is %d\n", ACK, packet.acknum);
	}

}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
	printf("\n----Timer interrupt has occured.----\n----A will resend packet with sequence number: %d----\n", buffedPacketA.seqnum);
	tolayer3(A, buffedPacketA);
	startTimer(A, TIMERVAL);

}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
	state_of_A = 0;
	ACK = 0;
	SEQ = 0;
}


/* 
 * Note that with simplex transfer from A-to-B, there is no routine  B_output() 
 */

/*
 * B_input(packet),where packet is a structure of type pkt. This routine 
 * will be called whenever a packet sent from the A-side (i.e., as a result 
 * of a tolayer3() being done by a A-side procedure) arrives at the B-side. 
 * packet is the (possibly corrupted) packet sent from the A-side.
 */
void B_input(struct pkt packet) {
	if (packet.seqnum == state_of_B && checkChksm(packet.seqnum, packet.acknum, packet.payload, sizeof(packet.payload), packet.checksum)) {
		struct msg thisMessage;
		struct pkt thisPacket;

		//strcpy(thisMessage.data, packet.payload);
		for (i = 0; i < 20; i++) {
			thisMessage.data[i] = packet.payload[i];
		}
		thisMessage.data[20] = 0;
		printf("\n----B received packet with sequence number: %d----\n", packet.seqnum);
		printf("----Payload of received packet is: %s----\n", thisMessage.data);
		// send message retrieved to layer 5 on B side
		tolayer5(B, thisMessage);

		state_of_B = complement(state_of_B);

		thisPacket.acknum = packet.seqnum;
		thisPacket.seqnum = thisPacket.acknum;

		for (i = 0; i < 20; i++) {
			thisPacket.payload[i] = packet.payload[i];
		}

		thisPacket.checksum = (int)calculateChksm(thisPacket.seqnum, thisPacket.acknum, thisPacket.payload, sizeof(thisPacket.payload));

		// keep a buffed packet of B's ACK packet
		buffedPacketB = thisPacket;

		printf("\n----B is sending packet which has ACK value: %d----\n", thisPacket.acknum);

		// send ACK packet to internet from B side
		tolayer3(B, thisPacket);

		// signifies that B got the proper packet
		gotpacket = 1;
	}
	else {
		printf("\n----B did not accept the packet which had sequence number: %d----\n", packet.seqnum);

		if (gotpacket == 1) {
			printf("----B is resending packet with ACK value: %d----\n", buffedPacketB.acknum);
			tolayer3(B, buffedPacketB);
		}

	}
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
	// NOT USED FOR ALTERNATING BIT
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
	state_of_B = 0;
	gotpacket = 0;
}

// function to simply complement(flip) the given value
int complement(int val) {
	if (val == 0) {
		return 1;
	}
	else {
		return 0;
	}
}

// calculates the checksum of the packet based on CRC32 algorithm.
uint32_t calculateChksm(uint32_t seqnum, uint32_t acknum, char* payload, int payloadlen) {
	int byte, index;
	uint8_t bit;
	uint32_t remainder = 0xFFFFFFFF;
	int datalen = payloadlen + 8;
	uint8_t* data = malloc((datalen)*sizeof(uint8_t));
	//divide acknum and seqnum into chunks of 8 bits and load into data array
	data[0] = (uint8_t) seqnum;
	data[1] = (uint8_t) (seqnum >> 8);
	data[2] = (uint8_t) (seqnum >> 16);
	data[3] = (uint8_t) (seqnum >> 24);
	data[4] = (uint8_t) acknum;
	data[5] = (uint8_t) (acknum >> 8);
	data[6] = (uint8_t) (acknum >> 16);
	data[7] = (uint8_t) (acknum >> 24);
	//load the rest of the data array with payload
	for (index = 8; index < datalen; index++) {
		data[index] = payload[index-8];
	}
	/*
	 * Perform modulo-2 division, a byte at a time.
	 */
	for (byte = 0; byte < datalen; ++byte)
	{
		/*
		 * Bring the next byte into the remainder.
		 */
		remainder ^= (data[byte] << (WIDTH - 8));
		/*
		 * Perform modulo-2 division, a bit at a time.
		 */
		for (bit = 8; bit > 0; --bit)
		{
			/*
			 * Try to divide the current data bit.
			 */
			if (remainder & TOPBIT)
			{
				remainder = (remainder << 1) ^ POLYNOMIAL;
			}
			else
			{
				remainder = (remainder << 1);
			}
		}
	}
	/*
	 * The final remainder is the CRC result.
	 */
	free(data);
	return (remainder);
}

// check the checksum
int checkChksm(uint32_t seqnum, uint32_t acknum, char* payload, int payloadlen, int checksum) {
	int byte, index;
	uint8_t bit;
	uint32_t remainder = 0xFFFFFFFF;
	int datalen = payloadlen + 8;
	uint8_t* data = malloc((datalen)*sizeof(uint8_t));
	//divide acknum and seqnum into chunks of 8 bits and load into data array
	data[0] = (uint8_t) seqnum;
	data[1] = (uint8_t) (seqnum >> 8);
	data[2] = (uint8_t) (seqnum >> 16);
	data[3] = (uint8_t) (seqnum >> 24);
	data[4] = (uint8_t) acknum;
	data[5] = (uint8_t) (acknum >> 8);
	data[6] = (uint8_t) (acknum >> 16);
	data[7] = (uint8_t) (acknum >> 24);
	//load the rest of the data array with payload
	for (index = 8; index < datalen; index++) {
		data[index] = payload[index-8];
	}
	/*
	 * Perform modulo-2 division, a byte at a time.
	 */
	for (byte = 0; byte < datalen; ++byte)
	{
		/*
		 * Bring the next byte into the remainder.
		 */
		remainder ^= (data[byte] << (WIDTH - 8));
		/*
		 * Perform modulo-2 division, a bit at a time.
		 */
		for (bit = 8; bit > 0; --bit)
		{
			/*
			 * Try to divide the current data bit.
			 */
			if (remainder & TOPBIT)
			{
				remainder = (remainder << 1) ^ POLYNOMIAL;
			}
			else
			{
				remainder = (remainder << 1);
			}
		}
	}
	/*
	 * The final remainder is the CRC result.
	 */
	free(data);
	if (remainder == checksum) {
		return 1;
	}
	else {
		return 0;
	}
}

