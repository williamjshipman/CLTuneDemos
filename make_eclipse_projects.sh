#! /bin/bash
cd ../CLTuneDemos_build
cmake -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE -DCMAKE_BUILD_TYPE=Debug ../CLTuneDemos
cd ../CLTuneDemos
