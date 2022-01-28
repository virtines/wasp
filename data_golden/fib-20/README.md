# fib-20

Recording the latencies (in cycles) from the perspective of the hypervisor of calculating fib(20) in various execution modes (16 bit, 32 bit, 64 bit). Data in files like 16-no.csv are not optimized by gcc, while data in 16-opt.csv are created from optimized code in gcc. This is measured as a latency from first vmrun to final hypercall signifying a result has been found
