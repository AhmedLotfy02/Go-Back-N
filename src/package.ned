package gobackn;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    parameters:
        double TD = 1.0;   
        double PT = 0.5;   
        double ED = 4.0;   
        double DD = 0.1; 
        double TO = 10.0;
        int WS=3;  
        int modification_random_number = 42;
        double LP = 0;
        
               
    submodules:
        coordinator: Coordinator {
            @display("p=335,47");
        }
        node0: Node {
            @display("p=119,156");
        }
        node1: Node {
            @display("p=623,151");
        }
    connections:
        coordinator.port0 <--> node0.coordinatorGate;
        coordinator.port1 <--> node1.coordinatorGate;
        node0.nodeGate <--> node1.nodeGate;
}

