each of the measurements in this directory are in cycles recorded from when the abstraction is called to when it "returns". In the case of process abstraction, we just fork and wait for the process to return doing the minimum work in the process as possible. In the thread abstraction it is a simple thread creation which calls a function then we join with the thread and record the total time. The function call abstraction is just recording how long it takes to return a value.

Each of the vm measurements are warm/cold measurements of how long it takes for a vm to be launched and immediately exit in cycles