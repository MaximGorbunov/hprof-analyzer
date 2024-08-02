# CLI Tool for Java old-gen heap analysis

### Build
```
 mkdir -p release && cd release && cmake -DCMAKE_BUILD_TYPE=Release .. && cd .. && cmake --build ./release --target hprof_analyzer
```
Output file will be at release/build/hprof_analyzer
### Usage
#### Take heap dump
```
jmap -dump:live,format=b,file=<heap_dump_file>
```
#### Take VM info
```
jcmd <pid> VM.info > log.txt
```
#### Use analyzer
```
heap_analyzer [-l <logFile>] [-h <heapFile>]

-l, --log-file <logFile> The log file containing old-gen merged object sizes with own and outgoing references size.
-h, --heap-file <heapFile> The heap dump file in HPROF format.
```
### Output
The tool processes the provided log and heap files to generate a list of old-gen merged object sizes with own and outgoing references size. Each line in the output represents an object in the heap and contains the following information:
```
<class_name> own size: <own_size> outgoing size: <outgoing_size> total size: <total_size>
```
Where:

**<class_name>** class name 

**<own_size>** is the size of the object's own size of fields

**<outgoing_size>** is the size of the object from this object from other objects in the heap.

**<total_size>** is the total size of the object, which includes both own and outgoing sizes.
