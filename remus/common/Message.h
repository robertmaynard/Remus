//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#ifndef __remus_common_Message_h
#define __remus_common_Message_h

#include <boost/shared_ptr.hpp>

#include <remus/common/zmqHelper.h>
#include <remus/common/MeshIOType.h>

namespace remus{
namespace common{
class Message
{
public:
  //pass in a data string the job message will copy and send
  Message(remus::common::MeshIOType mtype, SERVICE_TYPE stype, const std::string& data);

  //pass in a data pointer that the message will use when sending
  //the pointer data can't become invalid before you call send.
  Message(remus::common::MeshIOType mtype, SERVICE_TYPE stype, const char* data, int size);

  //creates a job message with no data
  Message(remus::common::MeshIOType mtype, SERVICE_TYPE stype);

  //creates a job message from reading in the socket
  explicit Message(zmq::socket_t& socket);

  bool send(zmq::socket_t& socket) const;
  void releaseData() { this->Data = NULL; this->Size = 0;}

  const remus::common::MeshIOType& MeshIOType() const { return MType; }
  const remus::SERVICE_TYPE& serviceType() const { return SType; }

  const char* const data() const { return Data; }
  int dataSize() const { return Size; }

  bool isValid() const { return ValidMsg; }

  template<typename T>
  void dump(T& t) const;

private:
  //a special struct that holds any space we need malloced
  struct DataStorage
    {
    DataStorage(): Size(0), Space(NULL){}
    DataStorage(unsigned int size): Size(size){this->Space = new char[this->Size];}
    ~DataStorage(){if(Size>0){delete this->Space;}}
    int Size;
    char* Space;
    };

  remus::common::MeshIOType MType;
  remus::SERVICE_TYPE SType;
  const char* Data; //points to the message contents to send
  int Size;
  bool ValidMsg; //tells if the message is valid, mainly used by the server

  //Storage is an optional allocation that is used when recieving job messages
  //when the process accepting the message has no memory already allocated to recieve it
  //It isn't used when sending messages to the server
  boost::shared_ptr<DataStorage> Storage;
};

//------------------------------------------------------------------------------
Message::Message(remus::common::MeshIOType mtype, SERVICE_TYPE stype, const std::string& data):
  MType(mtype),
  SType(stype),
  Data(NULL),
  Size(data.size()),
  ValidMsg(true),
  Storage(new DataStorage(data.size()))
  {
  //copy the string into local held storage, the string passed in can
  //be temporary, so we want to copy it.
  memcpy(this->Storage->Space,data.data(),this->Size);
  this->Data = this->Storage->Space;
  }


//------------------------------------------------------------------------------
Message::Message(remus::common::MeshIOType mtype, SERVICE_TYPE stype, const char* data, int size):
  MType(mtype),
  SType(stype),
  Data(data),
  Size(size),
  ValidMsg(true),
  Storage(new DataStorage())
  {
  }

//------------------------------------------------------------------------------
Message::Message(remus::common::MeshIOType mtype, SERVICE_TYPE stype):
  MType(mtype),
  SType(stype),
  Data(NULL),
  Size(0),
  ValidMsg(true),
  Storage(new DataStorage())
  {
  }

//------------------------------------------------------------------------------
Message::Message(zmq::socket_t &socket)
{
  //we are receiving a multi part message
  //frame 0: REQ header / attachReqHeader does this
  //frame 1: Mesh Type
  //frame 2: Service Type
  //frame 3: Job Data //optional
  zmq::more_t more;
  size_t more_size = sizeof(more);
  socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);

  //construct a job message from the socket
  zmq::removeReqHeader(socket);

  zmq::message_t MeshIOType;
  zmq::blocking_recv(socket,&MeshIOType);
  this->MType = *(reinterpret_cast<remus::common::MeshIOType*>(MeshIOType.data()));

  zmq::message_t servType;
  zmq::blocking_recv(socket,&servType);
  this->SType = *(reinterpret_cast<SERVICE_TYPE*>(servType.data()));

  socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);
  if(more>0)
    {
    zmq::message_t data;
    zmq::blocking_recv(socket,&data);
    this->Size = data.size();
    this->Storage.reset(new DataStorage(this->Size));

    memcpy(this->Storage->Space,data.data(),this->Size);
    this->Data = this->Storage->Space;
    }
  else
    {
    this->Size = 0;
    this->Data = NULL;
    }

  //see if we have more data. If so we need to say we are invalid
  //as we parsed the wrong type of message
  socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);
  ValidMsg=(more==0)?true:false;
}

//------------------------------------------------------------------------------
bool Message::send(zmq::socket_t &socket) const
  {
  //we are sending our selves as a multi part message
  //frame 0: REQ header / attachReqHeader does this
  //frame 1: Mesh Type
  //frame 2: Service Type
  //frame 3: Job Data //optional

  if(!this->ValidMsg)
    {
    return false;
    }
  zmq::attachReqHeader(socket);

  zmq::message_t MeshIOType(sizeof(this->MType));
  memcpy(MeshIOType.data(),&this->MType,sizeof(this->MType));
  zmq::blocking_send(socket,MeshIOType,ZMQ_SNDMORE);

  zmq::message_t service(sizeof(this->SType));
  memcpy(service.data(),&this->SType,sizeof(this->SType));
  if(this->Size> 0)
    {
    //send the service line not as the last line
    zmq::blocking_send(socket,service,ZMQ_SNDMORE);

    zmq::message_t data(this->Size);
    memcpy(data.data(),this->Data,this->Size);

    zmq::blocking_send(socket,data);
    }
  else //we are done
    {
    zmq::blocking_send(socket,service);
    }
  return true;
  }

//------------------------------------------------------------------------------
template<typename T>
void Message::dump(T& t) const
  {
  //dump the info to the t stream
  t << "Valid: " << this->isValid() << std::endl;
  t << "Mesh Type: " << this->MeshIOType() << std::endl;
  t << "Serivce Type: " << this->serviceType() << std::endl;
  t << "Size: " << this->dataSize() << std::endl;
  if(this->dataSize() > 0)
    {
    t << "Data: " << std::string(this->Data,this->dataSize()) << std::endl;
    }
  }
}
}
#endif //__remus_Message_h
