# Full-drinking-philosophers-problem-in-shared-memory

Dining Philosophers Problem, introduced by Edsger Dijkstra in 1972, is a classic problem in synchronization. In 1984, [Chandy and Misra](https://dl.acm.org/citation.cfm?doid=1780.1804) came up with an elegant solution to this problem. They generalized their solution to arbitrary conflict graphs — not just the 5-node cycle of Dijkstra’s original problem. They call their generalization the “Drinking Philosophers Problem.” However, their solution is not presented in shared-memory terms.

This program adapts the [Chandy and Misra solution] to shared memory and solve the full drinking philosophers problem. 

In this program, philosopers(threads) do not send any messages to each other. The only information each philosopher has is the states of all bottles that he/she needs for drinking (adjacent bottles). Each bottle has two possible states: dirty or clean (not dirty). 

Initially, all bottles are dirty. A philosopher who becomes hungry will request all adjacent bottles. He can only start drinking after he completes all his requests and get all the bottles he needs. When a philosopher requests a bottle, he will first check its state. If the bottle is dirty, he will update the state to clean. If it’s not dirty, he will have to wait on the condition variable associated with that bottle until he gets notified. Once he gets notified, he will check the state of the bottle again to make sure it’s not a spurious wakeup and then repeat the above process. Once a philosopher finishes drinking, he will update the states of all his adjacent bottles to dirty and notify all potential waiting philosophers. Those notified philosophers can continue requesting for the bottles he needs. 

This method prevents data races in shared memeory. For more algorithm details please refer to the paper mentioned above.
