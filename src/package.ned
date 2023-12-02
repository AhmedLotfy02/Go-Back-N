package gobackn;

@license(LGPL);
//
// TODO documentation
//
network Network
{
    @display("bgb=744,327");
    submodules:
        coordinator: Coordinator {
            @display("p=335,47");
        }
        sender: Node {
            @display("p=94,130");
        }
        receiver: Node {
            @display("p=567,130");
        }
    connections:
        coordinator.port0 <--> sender.coordinatorGate;
        coordinator.port1 <--> receiver.coordinatorGate;
        sender.nodeGate <--> receiver.nodeGate;
}

