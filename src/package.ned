package gobackn;

@license(LGPL);
//
// TODO documentation
//
network Network
{       
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

