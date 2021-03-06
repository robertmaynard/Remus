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

#include <remus/client/Client.h>
#include <remus/client/ServerConnection.h>
#include <remus/server/PortNumbers.h>
#include <remus/testing/Testing.h>

#include <remus/proto/zmqHelper.h>

#include <string>

namespace {

zmq::socketInfo<zmq::proto::tcp> make_tcp_socket(std::string host, int port)
{
  return zmq::socketInfo<zmq::proto::tcp>(host,port);
}

zmq::socketInfo<zmq::proto::ipc> make_ipc_socket(std::string host)
{
  return zmq::socketInfo<zmq::proto::ipc>(host);
}

zmq::socketInfo<zmq::proto::inproc> make_inproc_socket(std::string host)
{
  return zmq::socketInfo<zmq::proto::inproc>(host);
}

void verify_server_connection_inproc()
{
  remus::client::ServerConnection default_sc;
  remus::client::Client default_client(default_sc);
  const remus::client::ServerConnection& sc = default_client.connection();

  REMUS_ASSERT( (sc.endpoint().size() > 0) );
  const std::string default_endpoint = sc.endpoint();

  zmq::socketInfo<zmq::proto::tcp> default_socket("127.0.0.1",
                                                  remus::server::CLIENT_PORT);


  REMUS_ASSERT( (sc.endpoint() == default_socket.endpoint()) );

  zmq::socketInfo<zmq::proto::inproc> inproc_info("foo_inproc");
  remus::client::ServerConnection inproc_conn =
         remus::client::make_ServerConnection(inproc_info.endpoint());

  //to properly connect to an inproc_socket you first have something
  //bind to the name, otherwise the connecting socket just hangs. Also
  //the same zmq context needs to be shared between the binding and connecting
  //socket.
  zmq::socket_t inproc_bound_socket( *(inproc_conn.context()), ZMQ_REP );

  //need to use zmq::socketInfo with bindToAddress
  zmq::bindToAddress(inproc_bound_socket, inproc_info );

  remus::client::Client inproc_client(inproc_conn);

  REMUS_ASSERT( (inproc_client.connection().endpoint() ==
                 make_inproc_socket("foo_inproc").endpoint()) );

  //share a context and connection info between two client
  remus::client::Client inproc_client2(inproc_client.connection());

  REMUS_ASSERT( (inproc_client2.connection().endpoint() ==
                 make_inproc_socket("foo_inproc").endpoint()) );
  REMUS_ASSERT( (inproc_client.connection().context() ==
                 inproc_client2.connection().context()) );

  //test local host bool with tcp ip
  REMUS_ASSERT( (sc.isLocalEndpoint()==true) );
  REMUS_ASSERT( (inproc_client.connection().isLocalEndpoint()==true) );
}

void verify_server_connection_ipc()
{
  remus::client::ServerConnection default_sc;
  remus::client::Client default_client(default_sc);
  const remus::client::ServerConnection& sc = default_client.connection();

  REMUS_ASSERT( (sc.endpoint().size() > 0) );
  const std::string default_endpoint = sc.endpoint();

  zmq::socketInfo<zmq::proto::tcp> default_socket("127.0.0.1",
                                                  remus::server::CLIENT_PORT);


  REMUS_ASSERT( (sc.endpoint() == default_socket.endpoint()) );

  remus::client::ServerConnection ipc_conn =
        remus::client::make_ServerConnection("ipc://foo_ipc");
  remus::client::Client ipc_client(ipc_conn);

  REMUS_ASSERT( (ipc_client.connection().endpoint() ==
                 make_ipc_socket("foo_ipc").endpoint()) );

  //test local host bool with tcp ip
  REMUS_ASSERT( (sc.isLocalEndpoint()==true) );
  REMUS_ASSERT( (ipc_client.connection().isLocalEndpoint()==true) );
}

void verify_server_connection_tcp()
{
  remus::client::ServerConnection default_sc;
  remus::client::Client default_client(default_sc);
  const remus::client::ServerConnection& sc = default_client.connection();

  REMUS_ASSERT( (sc.endpoint().size() > 0) );
  const std::string default_endpoint = sc.endpoint();

  zmq::socketInfo<zmq::proto::tcp> default_socket("127.0.0.1",
                                                  remus::server::CLIENT_PORT);


  REMUS_ASSERT( (sc.endpoint() == default_socket.endpoint()) );

  remus::client::ServerConnection remote_tcp_conn =
        remus::client::make_ServerConnection("tcp://74.125.30.106:83");
  remus::client::Client remote_tcp_client(remote_tcp_conn);

  REMUS_ASSERT( (remote_tcp_client.connection().endpoint() ==
                 make_tcp_socket("74.125.30.106",83).endpoint()) );

  //test local host bool with tcp ip
  REMUS_ASSERT( (sc.isLocalEndpoint()==true) );
  REMUS_ASSERT( (remote_tcp_client.connection().isLocalEndpoint()==false) );
}

} //namespace


int UnitTestClient(int, char *[])
{
  verify_server_connection_inproc();
  #ifndef _WIN32
  verify_server_connection_ipc();
  #endif
  verify_server_connection_tcp();
  return 0;
}
