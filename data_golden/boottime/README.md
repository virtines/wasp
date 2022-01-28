T/2b2b/3030he experiments were done on lab desktop. 
qemu configs for nautilus comparison: -smp 4 -m 2G -nographic -serial stdio -monitor /dev/null

For OSv, we used the default script to run it:" ./scripts/run.py --verbose"



kvm.csv hyperv.csv
KVM was gathered on tinker2
- HyperV was gathered on the windows host
- Columns are how long (in cylces) it took to get from the previous columns to the new states

echo-server.csv
- VM is created for every tcp connection and has hypercalls for recv and send (just to that one connection)
- measured on tinker 2 with kvm.
- measured using rdtsc
- Data is absolute. Feel free to remove the PAE column as kmain() is the important measurement. (we record granular boots in kvm.csv and hyperv.csv)

