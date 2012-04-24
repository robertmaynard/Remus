
add_external_project(meshserver
  DEPENDS boost zeroMQ
  CMAKE_ARGS
    -DBOOST_ROOT:PATH=${BOOST_ROOT}
    -DZeroMQ_ROOT_DIR:FILEPATH=<INSTALL_DIR>
    -DCMAKE_INSTALL_PREFIX:Path=<INSTALL_DIR>
    )
