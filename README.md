# lirs-cache - a program that implements LIRS replacement policy

### Algorithm implemented according to its description in the paper: 
Song Jiang and Xiaodong Zhang. 2002. LIRS: An Efficient Low InterReference Recency Set Replacement Policy to Improve Buffer Cache
Performance. In Proceedings of the 2002 ACM SIGMETRICS International Conference on Measurement and Modeling of Computer Systems
(SIGMETRICS ’02). 31–42

### Installing and running

Prerequisites:
1) gtest should be installed on your system
2) cmake should be installed on your system

1. Create an empty folder where you want to build your project (f.e. it is called build)
2. `cd build`
3. run `cmake . `
4. run `make` or `cmake --build`

After following this simple set of instructions all apps are now in `build/apps` in each separate directory. All the tests lie in a separate folder in build called tests.
