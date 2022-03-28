echo "home" | yarp rpc /clipMaker/rpc 

# Run all body behaviors
echo "beh body 0 1" | yarp rpc /clipMaker/rpc 
echo "beh body 0 2" | yarp rpc /clipMaker/rpc 
echo "beh body 1 0" | yarp rpc /clipMaker/rpc 
echo "beh body 1 2" | yarp rpc /clipMaker/rpc 
echo "beh body 2 0" | yarp rpc /clipMaker/rpc 
echo "beh body 2 1" | yarp rpc /clipMaker/rpc 

echo "home" | yarp rpc /clipMaker/rpc 

# Run the one speech behavior (twice for safety)
echo "beh spch 0 1" | yarp rpc /clipMaker/rpc 
echo "beh spch 0 2" | yarp rpc /clipMaker/rpc 

echo "home" | yarp rpc /clipMaker/rpc 

# Run all gaze behaviors
echo "beh gaze 0 1" | yarp rpc /clipMaker/rpc 
echo "beh gaze 0 2" | yarp rpc /clipMaker/rpc 
echo "beh gaze 1 0" | yarp rpc /clipMaker/rpc 
echo "beh gaze 1 2" | yarp rpc /clipMaker/rpc 
echo "beh gaze 2 0" | yarp rpc /clipMaker/rpc 
echo "beh gaze 2 1" | yarp rpc /clipMaker/rpc 

echo "home" | yarp rpc /clipMaker/rpc 

# Run all expression behaviors.
echo "beh expr 0 1" | yarp rpc /clipMaker/rpc 
echo "beh expr 0 2" | yarp rpc /clipMaker/rpc 
echo "beh expr 1 0" | yarp rpc /clipMaker/rpc 
echo "beh expr 1 2" | yarp rpc /clipMaker/rpc 
echo "beh expr 2 0" | yarp rpc /clipMaker/rpc 
echo "beh expr 2 1" | yarp rpc /clipMaker/rpc 

echo "home" | yarp rpc /clipMaker/rpc 
