#include <stdio.h>

/* ******************************************************************
ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1 J.F.Kurose

This code should be used for PA2, unidirectional or bidirectional
data transfer protocols (from A to B. Bidirectional transfer of data
is for extra credit and is not required). Network properties:
- one way network delay averages five time units (longer if there
are other messages in the channel for GBN), but can be larger
- packets can be corrupted (either the header or the data portion)
or lost, according to user-defined probabilities
- packets will be delivered in the order in which they were sent
(although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0 /* change to 1 if you're doing extra credit */
/* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer */
/* 4 (students' code). It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities. */
struct msg
{
    char data[21];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code). Note the pre-defined packet structure, which all */
/* students must follow. */
struct pkt
{
    int seqnum;
    int acknum;
    int checksum;
    char payload[21];
};


// A sender
struct Sender {
    int base;
    int nextseq;
    int window_size;
    int buffer_index;
    struct pkt buffer[50]; //Buffer for 50 packets/messages
    float est_rtt;
} A;
// B receiver
struct Receiver {
    int expectseq;
    struct pkt ack_packet;
    struct pkt last_in_order_pkt;
} B;

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
A_init()
{
    A.base = 0;
    A.nextseq = 0;
    A.window_size = 8; //default size 8
    A.buffer_index = 0;
    A.est_rtt = 10; // est_rtt is 5 when no other messages are in the "medium"
}
B_init()
{
    B.expectseq = 0;
    B.ack_packet.acknum = 0;
    B.ack_packet.seqnum = -1; //
    B.last_in_order_pkt.seqnum = -1;
    memset(B.ack_packet.payload,0,20); //init packet payload to all zeros
}


/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
/*
TODO:
* Not sure if timer start/stop is correct
*/

/* called from layer 5, passed the data to be sent to other side */
A_output(message) struct msg message;
{
    /* where message is a structure of type msg, containing data to be sent to the
    B-side. This routine will be called whenever the upper layer at the sending side (A) has a message
    to send. It is the job of your protocol to ensure that the data in such a message is delivered in-order,
    and correctly, to the receiving side upper layer. */
    printf("\nA_OUTPUT");
    // Add null character to end of message
    message.data[20] = '\0';

    // Create packet
    struct pkt my_pkt;
    my_pkt.seqnum = A.nextseq;
    my_pkt.acknum = 0;
    my_pkt.checksum = create_checksum(message.data);
    strncpy(my_pkt.payload, message.data, 20);

    // Add packet to buffer
    A.buffer[A.buffer_index] = my_pkt;
    while(A.nextseq <= A.base) {
        // Start timer
        // if(A.nextseq == 0)
        starttimer(0, 10.0);

        // Send packet to reciever
        tolayer3(0, A.buffer[A.buffer_index]);

        // Increment sequence number and decrement buffer index
        A.nextseq = A.nextseq + 1;
        A.buffer_index = A.buffer_index - 1;
    }
    // Increment buffer index for next loop
    A.buffer_index += 1;
}

/* called from layer 3, when a packet arrives for layer 4 at B*/
B_input(packet) struct pkt packet;
{
    /* where packet is a structure of type pkt. This routine will be called whenever
    a packet sent from the A-side (i.e., as a result of a tolayer3() being done by a A-side procedure)
    arrives at the B-side. packet is the (possibly corrupted) packet sent from the A-side */
    printf("\nB_INPUT");
    
    int checksum = create_checksum(packet.payload);
    // If checksum is OK then send to layer 5 and send ACK
    if(packet.checksum == checksum && packet.seqnum == B.expectseq) {
        // Deliver to layer 5
        tolayer5(1, packet.payload);
        // Send ACK
        tolayer3(1, packet);
        // Increment expected sequence number
        B.expectseq = B.expectseq + 1;
        // Update last_in_order_packet
        B.last_in_order_pkt = packet;
    } else {
        printf("\nChecksum mismatch or sequence number out of order\n");
        // Send last in order packet if something went wrong. The new packet is dropped
        tolayer3(1, B.last_in_order_pkt);
    }

}


/* called from layer 3, when a packet arrives for layer 4 */
A_input(ack) struct pkt ack;
{
    /* where packet is a structure of type pkt. This routine will be called whenever
    a packet sent from the B-side (i.e., as a result of a tolayer3() being done by a B-side procedure)
    arrives at the A-side. packet is the (possibly corrupted) packet sent from the B-side. */
    printf("\nA_INPUT");

    // Recieve ACK and check sequence number
    if(A.base == ack.seqnum) {
        printf("\nStop\n");
        stoptimer(0);
        A.base = A.base + 1;
    } else {
        // Start timer 
        // printf("\nStart\n");
        // starttimer(0, 6.0);
    }
}



/* called when A's timer goes off */
A_timerinterrupt()
{
    /* This routine will be called when A's timer expires (thus generating a timer
    interrupt). You'll probably want to use this routine to control the retransmission of packets.
    See starttimer() and stoptimer() below for how the timer is started and stopped. */
    printf("\nA_TIMER_INTERRUPT\n");

    
    starttimer(0, 10.0);
    /* retransmission window is determined by base and nextseq
    - loop until base == nextseq
    */
    int i = A.base;
    for (i; i <= A.nextseq; ++i) {
        printf("retransmit\n");
        //find packet at index
        struct pkt *packet = &A.buffer[i % 50];
        //re-transmit packet to layer 3
        tolayer3(0, *packet);
    }
    
    
}
/* called when B's timer goes off */
B_timerinterrupt()
{
    printf("\nB_TIMER_INTERRUPT\n");
}


create_checksum(char *string) {
    // printf("\n%s\n",string);

    int sum = 0;
    int checksum;
    for(int i = 0; i < strlen(string); i++) {
        sum += string[i];
    }

    int key = 17;

    checksum = sum % key;
    
    return checksum;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */
/* need be completed only for extra credit */
B_output(message) struct msg message;{}

/****************************************************************
 * API provided for us to call found on LINE 376
 * starttimer(calling_entity,increment) 
    where calling_entity is either 0 (for starting the A-side
    timer) or 1 (for starting the B side timer), and increment is a float value indicating the amount of
    time that will pass before the timer interrupts. A's timer should only be started (or stopped) by A-
    side routines, and similarly for the B-side timer. To give you an idea of the appropriate increment
    value to use: a packet sent into the network takes an average of 5 time units to arrive at the other
    side when there are no other messages in the medium.
 * stoptimer(calling_entity) 
    where calling_entity is either 0 (for stopping the A-side timer) or
    1 (for stopping the B side timer).
 * tolayer3(calling_entity,packet) 
    where calling_entity is either 0 (for the A-side send) or 1
    (for the B side send), and packet is a structure of type pkt. Calling this routine will cause the
    packet to be sent into the network, destined for the other entity.
 * tolayer5(calling_entity,message) 
    where calling_entity is either 0 (for A-side delivery to
    layer 5) or 1 (for B-side delivery to layer 5), and message is a structure of type msg. With
    unidirectional data transfer, you would only be calling this with calling_entity equal to 1
    (delivery to the B-side). Calling this routine will cause data to be passed up to layer 5
 * /

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
- emulates the tranmission and delivery (possibly with bit-level corruption
and packet loss) of packets across the layer 3/4 interface
- handles the starting/stopping of a timer, and generates timer
interrupts (resulting in calling students timer handler).
- generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW. YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW. If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event
{
    float evtime;       /* event time */
    int evtype;         /* event type code */
    int eventity;       /* entity where event occurs */
    struct pkt *pktptr; /* ptr to packet (if any) assoc w/ this event */
    struct event *prev;
    struct event *next;
};
struct event *evlist = NULL; /* the event list */

/* possible events: */
#define TIMER_INTERRUPT 0
#define FROM_LAYER5 1
#define FROM_LAYER3 2

#define OFF 0
#define ON 1
#define A 0
#define B 1

int TRACE = 1;   /* for my debugging */
int nsim = 0;    /* number of messages from 5 to 4 so far */
int nsimmax = 0; /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;    /* probability that a packet is dropped */
float corruptprob; /* probability that one bit is packet is flipped */
float lambda;      /* arrival rate of messages from layer 5 */
int ntolayer3;     /* number sent into layer 3 */
int nlost;         /* number lost in media */
int ncorrupt;      /* number corrupted by media*/

main()
{
    struct event *eventptr;
    struct msg msg2give;
    struct pkt pkt2give;

    int i, j;
    char c;

    init();
    A_init();
    B_init();

    while (1)
    {
        eventptr = evlist; /* get next event to simulate */
        if (eventptr == NULL)
            goto terminate;
        evlist = evlist->next; /* remove this event from event list */
        if (evlist != NULL)
            evlist->prev = NULL;
        if (TRACE >= 2)
        {
            printf("\nEVENT time: %f,", eventptr->evtime);
            printf(" type: %d", eventptr->evtype);
            if (eventptr->evtype == 0)
                printf(", timerinterrupt ");
            else if (eventptr->evtype == 1)
                printf(", fromlayer5 ");
            else
                printf(", fromlayer3 ");
            printf(" entity: %d\n", eventptr->eventity);
        }
        time = eventptr->evtime; /* update time to next event time */
        if (nsim == nsimmax)
            break; /* all done with simulation */
        if (eventptr->evtype == FROM_LAYER5)
        {
            generate_next_arrival(); /* set up future arrival */
            /* fill in msg to give with string of same letter */
            j = nsim % 26;
            for (i = 0; i < 20; i++)
                msg2give.data[i] = 97 + j;
            if (TRACE > 2)
            {
                printf(" MAINLOOP: data given to student: ");
                for (i = 0; i < 20; i++)
                    printf("%c", msg2give.data[i]);
                printf("\n");
            }
            nsim++;
            if (eventptr->eventity == A)
                A_output(msg2give);
            else
                B_output(msg2give);
        }
        else if (eventptr->evtype == FROM_LAYER3)
        {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i = 0; i < 20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
            if (eventptr->eventity == A) /* deliver packet by calling */
                A_input(pkt2give);       /* appropriate entity */
            else
                B_input(pkt2give);
            free(eventptr->pktptr); /* free the memory for packet */
        }
        else if (eventptr->evtype == TIMER_INTERRUPT)
        {
            if (eventptr->eventity == A)
                A_timerinterrupt();
            else
                B_timerinterrupt();
        }
        else
        {
            printf("INTERNAL PANIC: unknown event type \n");
        }
        free(eventptr);
    }

terminate:
    printf(" \nSimulator terminated at time %f\n after sending %d msgs from layer5\n", time, nsim);
}

init() /* initialize the simulator */
{
    int i;
    float sum, avg;
    float jimsrand();

    printf("----- Stop and Wait Network Simulator Version 1.1 -------- \n\n");
    printf("Enter the number of messages to simulate: ");
    // scanf("%d", &nsimmax);
    nsimmax = 10;
    printf("%d\n", nsimmax);

    printf("Enter packet loss probability [enter 0.0 for no loss]:");
    // scanf("%f", &lossprob);
    lossprob = 0.2;
    printf("%f\n", lossprob);

    printf("Enter packet corruption probability [0.0 for no corruption]:");
    // scanf("%f", &corruptprob);
    corruptprob = 0.0;
    printf("%f\n", corruptprob);

    printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
    // scanf("%f", &lambda);
    lambda = 3.0;
    printf("%f\n", lambda);

    printf("Enter TRACE:");
    scanf("%d", &TRACE);

    srand(9999); /* init random number generator */
    sum = 0.0;   /* test random number generator for students */
    for (i = 0; i < 1000; i++)
        sum = sum + jimsrand(); /* jimsrand() should be uniform in [0,1] */
    avg = sum / 1000.0;
    if (avg < 0.25 || avg > 0.75)
    {
        printf("It is likely that random number generation on your machine\n");
        printf("is different from what this emulator expects. Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(0);
    }

    ntolayer3 = 0;
    nlost = 0;
    ncorrupt = 0;

    time = 0.0;              /* initialize time to 0.0 */
    generate_next_arrival(); /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1]. The routine below is used to */
/* isolate all random number generation in one location. We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm] */
/****************************************************************************/
float jimsrand()
{
    double mmm = 2147483647; /* largest int - MACHINE DEPENDENT!!!!!!!! */
    float x;                 /* individual students may need to change mmm */
    x = rand() / mmm;        /* x should be uniform in [0,1] */
    // printf("%f\n", x);
    return (x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/* The next set of routines handle the event list */
/*****************************************************/

generate_next_arrival()
{
    double x, log(), ceil();
    struct event *evptr;
    char *malloc();
    float ttime;
    int tempint;

    if (TRACE > 2)
        printf(" GENERATE NEXT ARRIVAL: creating new arrival\n");

    x = lambda * jimsrand() * 2; /* x is uniform on [0,2*lambda] */
    /* having mean of lambda */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + x;
    evptr->evtype = FROM_LAYER5;
    if (BIDIRECTIONAL && (jimsrand() > 0.5))
        evptr->eventity = B;
    else
        evptr->eventity = A;
    insertevent(evptr);
}

insertevent(p) struct event *p;
{
    struct event *q, *qold;

    if (TRACE > 2)
    {
        printf(" INSERTEVENT: time is %lf\n", time);
        printf(" INSERTEVENT: future time will be %lf\n", p->evtime);
    }
    q = evlist; /* q points to header of list in which p struct inserted */
    if (q == NULL)
    { /* list is empty */
        evlist = p;
        p->next = NULL;
        p->prev = NULL;
    }
    else
    {
        for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
            qold = q;
        if (q == NULL)
        { /* end of list */
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q == evlist)
        { /* front of list */
            p->next = evlist;
            p->prev = NULL;
            p->next->prev = p;
            evlist = p;
        }
        else
        { /* middle of list */
            p->next = q;
            p->prev = q->prev;
            q->prev->next = p;
            q->prev = p;
        }
    }
}

printevlist()
{
    struct event *q;
    int i;
    printf("--------------\nEvent List Follows:\n");
    for (q = evlist; q != NULL; q = q->next)
    {
        printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype, q->eventity);
    }
    printf("--------------\n");
}

/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
stoptimer(AorB) int AorB; /* A or B is trying to stop timer */
{
    struct event *q, *qold;

    if (TRACE > 2)
        printf(" STOP TIMER: stopping timer at %f\n", time);
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            /* remove this event */
            if (q->next == NULL && q->prev == NULL)
                evlist = NULL;        /* remove first and only event on list */
            else if (q->next == NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q == evlist)
            { /* front of list - there must be event after */
                q->next->prev = NULL;
                evlist = q->next;
            }
            else
            { /* middle of list */
                q->next->prev = q->prev;
                q->prev->next = q->next;
            }
            free(q);
            return;
        }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
}

starttimer(AorB, increment) int AorB; /* A or B is trying to stop timer */
float increment;
{

    struct event *q;
    struct event *evptr;
    char *malloc();

    if (TRACE > 2)
        printf(" START TIMER: starting timer at %f\n", time);
    /* be nice: check to see if timer is already started, if so, then warn */
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            printf("Warning: attempt to start a timer that is already started\n");
            return;
        }

    /* create future event for when timer goes off */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + increment;
    evptr->evtype = TIMER_INTERRUPT;
    evptr->eventity = AorB;
    insertevent(evptr);
}

/************************** TOLAYER3 ***************/
tolayer3(AorB, packet) int AorB; /* A or B is trying to stop timer */
struct pkt packet;
{
    struct pkt *mypktptr;
    struct event *evptr, *q;
    char *malloc();
    float lastime, x, jimsrand();
    int i;

    ntolayer3++;

    /* simulate losses: */
    if (jimsrand() < lossprob)
    {
        nlost++;
        if (TRACE > 0)
            printf(" TOLAYER3: packet being lost\n");
        return;
    }

    /* make a copy of the packet student just gave me since he/she may decide */
    /* to do something with the packet after we return back to him/her */
    mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
    mypktptr->seqnum = packet.seqnum;
    mypktptr->acknum = packet.acknum;
    mypktptr->checksum = packet.checksum;
    for (i = 0; i < 20; i++)
        mypktptr->payload[i] = packet.payload[i];
    if (TRACE > 2)
    {
        printf(" TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
               mypktptr->acknum, mypktptr->checksum);
        for (i = 0; i < 20; i++)
            printf("%c", mypktptr->payload[i]);
        printf("\n");
    }

    /* create future event for arrival of packet at the other side */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtype = FROM_LAYER3;      /* packet will pop out from layer3 */
    evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
    evptr->pktptr = mypktptr;         /* save ptr to my copy of packet */
    /* finally, compute the arrival time of packet at the other end.
    medium can not reorder, so make sure packet arrives between 1 and 10
    time units after the latest arrival time of packets
    currently in the medium on their way to the destination */
    lastime = time;
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == FROM_LAYER3 && q->eventity == evptr->eventity))
            lastime = q->evtime;
    evptr->evtime = lastime + 1 + 9 * jimsrand();

    /* simulate corruption: */
    if (jimsrand() < corruptprob)
    {
        ncorrupt++;
        if ((x = jimsrand()) < .75)
            mypktptr->payload[0] = 'Z'; /* corrupt payload */
        else if (x < .875)
            mypktptr->seqnum = 999999;
        else
            mypktptr->acknum = 999999;
        if (TRACE > 0)
            printf(" TOLAYER3: packet being corrupted\n");
    }

    if (TRACE > 2)
        printf(" TOLAYER3: scheduling arrival on other side\n");
    insertevent(evptr);
}

/************************** TOLAYER5 ***************/
tolayer5(AorB, datasent) int AorB;
char datasent[20];
{
    int i;
    if (TRACE > 2)
    {
        printf(" TOLAYER5: data received: ");
        for (i = 0; i < 20; i++)
            printf("%c", datasent[i]);
        printf("\n");
    }
}