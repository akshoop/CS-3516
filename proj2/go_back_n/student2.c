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
uint32_t calculateChksm(uint32_t, uint32_t, char*, int);
int checkChksm(uint32_t, uint32_t, char*, int, int);

// int i for loop
int i;
// top and bottom of buffer
int top, bottom;
// buffer containg unsent packet
struct pkt *buffer;
// the size of the buffer
int buffer_size = 50;
// the packet that B sends ACKs for
struct pkt ACK_packet;
// the window size, base value, next sequence number, and expected sequence
int window_size, base, nextseq, expected_sequence;

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
	int temp_top;
	message.data[20] = 0;

	// check if queue is full
	if (bottom != -1 && ((bottom + 1) % buffer_size == top)) {
		fprintf(stderr, "Error: queue is full\n");
		exit(1);
	}

	struct pkt thisPacket;

	for (i = 0; i < 20; i++) {
			thisPacket.payload[i] = message.data[i];
		}
	thisPacket.seqnum = nextseq;
	thisPacket.acknum = thisPacket.seqnum;
	thisPacket.checksum = (int)calculateChksm(thisPacket.seqnum, thisPacket.acknum, thisPacket.payload, sizeof(thisPacket.payload));

	// put created packet at end of buffer
	// we do modulus so that the values of top and bottom
	// never go beyond the buffer_size value
	bottom = (bottom + 1) % buffer_size;
	buffer[bottom] = thisPacket;

	// send packets that are in the buffer
	temp_top = (nextseq - base + top) % buffer_size;

	do {
		// send packet that's in top of buffer
		tolayer3(A, buffer[temp_top]);

		// if packet is first in buffer, start timer
		if (base == nextseq) {
			startTimer(A, TIMERVAL);
		}

		nextseq++;
		temp_top = (temp_top + 1) % buffer_size;


	} while (nextseq < (base + window_size) && temp_top != ((bottom + 1 % buffer_size)));
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

	// check if checksum matches
	if (checkChksm(packet.seqnum, packet.acknum, packet.payload, sizeof(packet.payload), packet.checksum)) {

		printf("\n----A has received the proper packet, with ACK value: %d----\n", packet.acknum);

		// now we move the top pointer
		if (packet.acknum >= base) {
			top = (top + (packet.acknum - base) + 1) % buffer_size;

			if ((bottom + 1) % buffer_size == top) {
				// we reset the top and bottom
				top = 0;
				bottom = -1;
			}
		}

		base = packet.acknum + 1;

		if (base == nextseq) {
			// stop A's timer
			stopTimer(A);
		}
		else {
			// start A's timer
			startTimer(A, TIMERVAL);
		}
	}

	else {
		printf("\n----A got corrupt packet.----\n");
	}

}

/*
 * A_timerinterrupt()  This routine will be called when A's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void A_timerinterrupt() {
	int amt_of_packets, temp_top;
	printf("\n----Timer interrupt has occured.----\n----A will resend packets----\n");

	amt_of_packets = nextseq - base;
	temp_top = top;

	// start A's timer again
	startTimer(A, TIMERVAL);

	// go through and resend the packets in buffer
	for (i = 0; i < amt_of_packets; i++) {
		tolayer3(A, buffer[temp_top]);
		temp_top = (temp_top + 1) % buffer_size;
	}
}  

/* The following routine will be called once (only) before any other    */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
	nextseq = base = 1;
	window_size = 8;
	top = 0;
	bottom = -1;
	buffer = (struct pkt * )malloc(sizeof(struct pkt) * buffer_size);
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
	if (packet.seqnum != expected_sequence && checkChksm(packet.seqnum, packet.acknum, packet.payload, sizeof(packet.payload), packet.checksum) == 0) {
		printf("----B did not accept packet which had sequence number: %d----\n", packet.seqnum);
		tolayer3(B, ACK_packet);
	}

	else {
		struct msg thisMessage;

		for (i = 0; i < 20; i++) {
			thisMessage.data[i] = packet.payload[i];
		}
		thisMessage.data[20] = 0;
		printf("\n----B received packet with sequence number: %d----\n", packet.seqnum);
		printf("----Payload of received packet is: %s----\n", thisMessage.data);
		// send message retrieved to layer 5 on B side
		tolayer5(B, thisMessage);

		// ACK packet related
		ACK_packet.acknum = packet.seqnum;
		ACK_packet.seqnum = ACK_packet.acknum;
		for (i = 0; i < 20; i++) {
			ACK_packet.payload[i] = packet.payload[i];
		}
		ACK_packet.checksum = (int)calculateChksm(ACK_packet.seqnum, ACK_packet.acknum, ACK_packet.payload, sizeof(ACK_packet.payload));
		tolayer3(B, ACK_packet);
		printf("\n----ACK packet sent----\n");
		// increment expected packet sequence number
		expected_sequence++;
	}	
}

/*
 * B_timerinterrupt()  This routine will be called when B's timer expires 
 * (thus generating a timer interrupt). You'll probably want to use this 
 * routine to control the retransmission of packets. See starttimer() 
 * and stoptimer() in the writeup for how the timer is started and stopped.
 */
void  B_timerinterrupt() {
	// NOT USED FOR GO-BACK-N
}

/* 
 * The following routine will be called once (only) before any other   
 * entity B routines are called. You can use it to do any initialization 
 */
void B_init() {
	// packet to send if first packet from A didn't go through
	ACK_packet.acknum = 0;
	ACK_packet.seqnum = 0;
	ACK_packet.checksum = (int)calculateChksm(ACK_packet.seqnum, ACK_packet.acknum, ACK_packet.payload, sizeof(ACK_packet.payload));

	expected_sequence = 1;
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

