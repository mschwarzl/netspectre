# NetSpectre
Proof-of-concept code for the NetSpectre [paper](https://martinschwarzl.at/media/files/netspectre.pdf).
See the setup details there on which CPUs we run the experiments.

All proof-of-concepts rely on UDS and UDP. 
For side-channel attacks using TCP see our work on [memory-compression attacks](https://github.com/IAIK/Memory-Compression-Attacks).


In each folder you have a client and a server code.
The make command creates you the client (attacker) and server (victim) binaries.
There are plot scripts to plot the cache hits and misses.
In the histogram folders you can create the histograms for cache resp. AVX warmed up or cooled down.

You can vary the number of repetitions using the constant REP.

Using the histograms you should be capable to choose appropriate thresholds to distinguish 
between a 0 and a 1.

### Leaking bytes

Mistraining: 10-15 valid requests (indices) should be enough to send at least one invalid index , which is speculatively executed.

We observed that interrupts improve the attacks success rate : 
`stress -d 1` or with `stress -i 1`

For notebook CPUs it might occur, that one AVX instruction is not enough to warm up the unit.
Thus, you need to warm it up a little stronger and using spectre make it "hot" and then measure the timing.

Also it might occur that the timings of AVX shift in a certain way a little around. 


## AVX_LFENCE_POC
After compiling, there are 3
binaries: leak, leak_baseline, and leak_lfence. All the PoCs measure the
execution time of an AVX2 instruction in two cases which are shown as
histograms in two columns.

- leak:
    Left column: An AVX2 instruction was executed speculatively before the measurement
    Right column: No AVX2 instruction was executed before the measurement
 
    Expected output: The histograms should differ.

- leak_baseline:
    Both columns: No AVX2 instruction was executed before the measurement
Expected output: The histograms should be roughly the same

- leak_lfence:
    Left column: An AVX2 instruction was executed speculatively AFTER an lfence instruction
    Right column: No AVX2 instruction was executed before the measurement
    
### Expected output: 
If lfence stops speculation, the histograms should be the same. However, the histograms differ and look similar as with the leak binary.

