# Data Link Layer Protocols Simulation

In this project, we develop, simulate and test data link layer protocols between two nodes that are connected with a noisy channel, where the transmission is not error-free, packets may get corrupted, duplicated, delayed, or lost, and the buffers are of limited sizes.

**The design of the system’s network:**

<img src="assets/network_design.png" alt="Network design" >

The system network topology is shown in the figure. It consists of one pair of nodes [Node0, Node1], and one coordinator that is connected to the pair.

In this project, the pair of nodes would communicate and exchange messages using the **Go Back N** algorithm with noisy channel and sender window of size WS, receiver window of size WR=1, using **Byte Stuffing** as a framing algorithm, and **checksum** as an error detection algorithm.

## System inputs:

1. Each node has a list of messages to send, and each node reads its list of messages from a different input text file; namely ‘input0.txt’ for Node0 and ‘input1.txt’ for Node1.

2. Each message starts in a new line, and there is a 4-bits binary prefix before each message. These 4-bits represent the possibility of [Modification, Loss, Duplication, Delay] that would affect this message. For example, “1010 Data Link” means that the message “Data Link” will have a modification to one of its bits while sending, and will be sent twice. Figure2 includes an example of the input file.

<img src="assets/input_example.png" alt="Input Example" > 

3. Table 1 contains the details of the errors and their priorities:

**Error code [Modification, Loss, Duplication, Delay]**   | **Effect**                                                               
----------------------------------------------------------|----------------------------------------------------------------------------
 **0000**                                                 | **No error**       
----------------------------------------------------------|----------------------------------------------------------------------------
 **0001**                                                 | **Delay**
                                                            For example, using the default delays (listed below in the
                                                            section “Delays in the system”):
                                                            @t=0 read line
                                                            @t=0.5 end processing time
----------------------------------------------------------|----------------------------------------------------------------------------                                                            
                                                            @t=5.5 the message is received.      
