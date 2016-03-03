## Using the Worker API ##

The worker interface for remus revolves around the ```remus::Worker``` class.
The worker uses an asynchronous strategy where the worker pushes information to the
server and doesn't wait for a response. Between these messages the worker
is sends heartbeat messages to the server.  This allows the server to monitor
workers and be able to inform client when workers crash. The worker supports
both blocking and non blocking ways of getting jobs. You should check the validity
of all jobs before processing them, as a client is able to terminate a job after
it is sent to you, but before you take it from the queue.

Putting this all together and show how to get a job from the server,
send back some status, and then return a result:

```cpp

//keep asking for job till we have a valid job
remus::worker::Job j = worker.getJob();
while( j.valid() )
  { //a job could be invalid if a client terminates it, or if the server
    //crashes and has told us to shutdown (in that case we also need) to
    //check why the job isn't valid
  j = worker.getJob();
  }

//lets take the data key's value out of the job submission
remus::proto::JobContent content;
const bool has_data_content = jd.details("data", content);

//extract the data from the content
std::string message;
if(has_data_content)
  {
  message = std::string(content.data(),content.dataSize());
  mesasge += " and ";
  }
message += "Hello Client I am currently in progress";

remus::proto::JobProgress progress(message);
remus::proto::JobStatus status(j.id(),progress);

for(int i=0; i < 100; i+=10)
  {
  progress.setValue(i+1);
  worker.updateStatus( status.updateProgress(progress) );
  }

std::string finished_message(content.data(),content.dataSize());
finished_message += " and Hello Client I am now finished";
remus::proto::JobResult results(j.id(),finished_message);
worker.returnResult(results);

```

### Server Connection ###
The server that the remus worker connects to is determined by the ```ServerConnection```
that is provided at construction of the worker. The ```ServerConnection``` by
default connects to a local remus server using tcp-ip. You can request a custom
location by doing one of the following:

```cpp
//You can explicitly state the machine name and port with tcp-ip by using one of the following:
remus::worker::ServerConnection conn("meshing_host", 8080);
remus::worker::ServerConnection conn_other = remus::worker::make_ServerConnection("tcp://74.125.30.106:84");

//You can also request a custom inproc or ipc connection by doing:
remus::worker::ServerConnection sc_inproc = remus::worker::make_ServerConnection("inproc://servers_workers");
remus::worker::ServerConnection sc_ipc = remus::worker::make_ServerConnection("ipc://servers_workers");
```


## Constructing a Remus Worker File ##

When you are creating custom workers that you want the default worker factory
to understand you need to generate a Remus worker file. The simplest type
of worker that doesn't specify a requirements file would look like:

```
{
"ExecutableName": "ExampleWorker",
"InputType": "Model",
"OutputType": "Mesh3D"
}
```

If you have a worker that has explicit job requirements from a file you can
specify it like so:

```
{
"ExecutableName": "ExampleWorker",
"InputType": "Model",
"OutputType": "Mesh3D",
"File": "<path>",
#valid options are JSON, BSON, XML and USER
"FileFormat" : "JSON"
}
```

It is also possible to specify environment variables that will be
set when the worker is started, command-line arguments passed to the
worker (in addition to the endpoint and any factory-wide arguments),
and a tag for the worker's JobRequirements (when the requirements
themselves are specified in a separate file -- if the worker must be
started to obtain its requirements, then it is responsible for
populating the tag field). Here is an example:
```
{
"ExecutableName": "ExampleWorker",
"InputType": "Model",
"OutputType": "Mesh3D",
"Tag":  {"thing":"yes"},
"Arguments":  ["-argtest","@SELF@"],
"Environment":  {
  "LD_LIBRARY_PATH":"/usr/local/smtk/lib"
  },
}
```
which specifies that the server should set the LD_LIBRARY_PATH
environment variable before executing the worker; and specifies that the
worker should be started with additional command line arguments. Note that
the first occurrence of "@SELF@" in any argument is converted into the
full path to the remus worker file (as identified by the WorkerFinder
class at run time, not at configure- or build-time).


Finally, the Tag string should evaluate to valid JSON or be doubly-escaped
as it is substituted into the worker file bare; that way parsing the worker
file generated by the above provides access to a dictionary rather than
a string. This is convenient for the FactoryFileParser and its subclasses.


## Calling an External Program ##

Calling some external command line program is common task for workers.
Remus has built in support for executing processes.

```
using remus::common::ExecuteProcess;
using remus::common::ProcessPipe;

ExecuteProcess lsProcess("ls");
lsProcess.execute(ExecuteProcess::Attached);

ProcessPipe data = lsProcess->poll(-1);
if(data.type == ProcessPipe::STDOUT)
  {
  std::cout << data.text << std::endl;
  }
```

### Thread Safety ###

Remus workers are not thread safe. Applications can not use a worker from multiple
threads unless they use their own full memory barrier locking mechanisms.

A Remus worker creates and starts threads on construction, so take that into
consideration when designing your system.

### Dynamic Polling ###
See [Server Readme][] for information related to dynamic polling.

[Server Readme]: remus/server/Readme.md