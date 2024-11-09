# dump
A C allocater, that is less error-prone and is memory safe.  
[Malwele] At the cost of your time and system resources i.e. CPU.  
It makes a single allocation and makes a table of entries for it, changing and allocating in a shared space.  
It is not good, because new allocations may cause the whole memory space to be moved and cause alot of down time.  
And lots of small holes are left after small allocations.  
Leading to a fragmented system, resource hungry and poor design.  
Don't use as it is bad(This is an experiment).  
