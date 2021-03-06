project(Remus_Server)

add_subdirectory(detail)

set(headers
    FactoryFileParser.h
    FactoryWorkerSpecification.h
    PortNumbers.h
    Server.h
    ServerPorts.h
    ThreadWorkerFactory.h
    WorkerFactory.h
    WorkerFactoryBase.h
    )

set(server_srcs
   detail/ActiveJobs.cxx
   detail/EventPublisher.cxx
   detail/JobQueue.cxx
   detail/SocketMonitor.cxx
   detail/WorkerFinder.cxx
   detail/WorkerPool.cxx
   FactoryFileParser.cxx
   FactoryWorkerSpecification.cxx
   Server.cxx
   ServerPorts.cxx
   ThreadWorkerFactory.cxx
   WorkerFactory.cxx
   WorkerFactoryBase.cxx
   )

#include cjson
include_directories("${Remus_SOURCE_DIR}/thirdparty/cJson/")

add_library(RemusServer ${server_srcs} ${headers})
target_link_libraries(RemusServer
                      LINK_PUBLIC RemusCommon
                      LINK_PRIVATE RemusProto
                                   remuscJSON
                                   ${Boost_LIBRARIES}
                                   ${CMAKE_THREAD_LIBS_INIT}
                                   )

#disable checked iterators in RemusServer
if(MSVC)
  target_compile_definitions(RemusServer PRIVATE _SCL_SECURE_NO_WARNINGS)
endif()

#create the export header symbol exporting
remus_export_header(RemusServer ServerExports.h)

#install the library
remus_install_library(RemusServer)

#install the headers
remus_public_headers(${headers})

#setup the exports for the library when used from a build tree
export(TARGETS RemusServer
               RemusProto
               RemusCommon
               remuscJSON
               FILE Remus-exports.cmake)

if(Remus_ENABLE_TESTING)
  target_link_libraries(TestBuild_remus_server
                        LINK_PRIVATE ${Boost_LIBRARIES} )
  add_subdirectory(testing)
endif()
