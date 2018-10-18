cmake_minimum_required(VERSION 3.4.1)

add_library( # Sets the name of the library.
             daemon
             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/com_cit_daemon_Daemon.cpp
             )

find_library( # Sets the name of the path variable.
              log-lib
              log )

target_link_libraries( # Specifies the target library.
                       daemon
                       ${log-lib} )