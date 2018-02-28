#Transactional NP-Heap

In the previous project, we have built NPHeap, an in-memory kernel module that provides efficient data sharing among different processes. Although NPHeap provides some degree of parallel programming support -- through simple, traditional locking/unlocking mechanism in either a fine-grained or coarse-grained manner, depending on how you implemented it, the performance of such a lock-based mechanism can be limited once the degree of parallelism increases. 

In this project, you will be investigating a potential design that supports parallel accesses among concurrent processes more efficiently. This design leverages the concept of "transactional memory". Instead of using locks, the programmer declares "atomic regions" in the transactional memory programming model. All memory accesses in the atomic region are considered as a single transaction. A transaction can commit, i.e. present its updates, to the underlying memory device only if no other transactions have changed any value the transaction depends on. If the transaction cannot commit, the transaction must be aborted and restarted.

In order to check if a transaction can be committed or not, one way is to maintain a "version" number for data stored in the memory device. Whenever your program start a transaction or say, entering an atomic region, you should obtain a "transaction id" for this ongoing transaction. At the end of the transaction, if the version of all values that the transaction read remains the same as the beginning of the transaction, meaning that the results generated from the transaction are still based on valid input data, the transaction can commit all the value it modified as a new version of the data to the memory device. Otherwise, the transaction should be aborted and restart from the beginning. 
