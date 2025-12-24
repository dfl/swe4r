cmake_minimum_required(VERSION 3.14)
project(SwissEphemerisDownloader)

# Include FetchContent module for downloading dependencies
include(FetchContent)

# Swiss Ephemeris version pinning
# Use v2.10.3a as the latest stable release
# Can be overridden via: cmake -DSWISSEPH_VERSION=<tag>
set(SWISSEPH_VERSION "v2.10.3a" CACHE STRING "Swiss Ephemeris version tag")

# Fetch Swiss Ephemeris from GitHub
FetchContent_Declare(
  swisseph
  GIT_REPOSITORY https://github.com/aloistr/swisseph.git
  GIT_TAG        ${SWISSEPH_VERSION}
  GIT_SHALLOW    TRUE  # Only fetch the specific tag, not full history
  SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/swisseph_src
)

message(STATUS "Fetching Swiss Ephemeris ${SWISSEPH_VERSION} from GitHub...")
FetchContent_MakeAvailable(swisseph)

# List of C source files to copy (exclude utility programs)
set(SWEPH_SOURCES
  swecl.c
  swedate.c
  sweephe4.c
  swehel.c
  swehouse.c
  swejpl.c
  swemmoon.c
  swemplan.c
  sweph.c
  swephlib.c
)

# Excluded files (utility programs, not part of library):
# swephgen4.c, swemini.c, sweasp.c, swevents.c, swetest.c

# List of header files to copy
set(SWEPH_HEADERS
  swedate.h
  swedll.h
  sweephe4.h
  swehouse.h
  swejpl.h
  swemptab.h
  swenut2000a.h
  sweodef.h
  sweph.h
  swephexp.h
  swephlib.h
  swevents.h
)

# Copy source files from fetched directory to ext/swe4r
foreach(src_file ${SWEPH_SOURCES})
  configure_file(
    ${swisseph_SOURCE_DIR}/${src_file}
    ${CMAKE_CURRENT_SOURCE_DIR}/${src_file}
    COPYONLY
  )
endforeach()

# Copy header files
foreach(hdr_file ${SWEPH_HEADERS})
  configure_file(
    ${swisseph_SOURCE_DIR}/${hdr_file}
    ${CMAKE_CURRENT_SOURCE_DIR}/${hdr_file}
    COPYONLY
  )
endforeach()

# Copy data files from root directory
configure_file(
  ${swisseph_SOURCE_DIR}/agpl-3.0.txt
  ${CMAKE_CURRENT_SOURCE_DIR}/agpl-3.0.txt
  COPYONLY
)

configure_file(
  ${swisseph_SOURCE_DIR}/seleapsec.txt
  ${CMAKE_CURRENT_SOURCE_DIR}/seleapsec.txt
  COPYONLY
)

# Copy sefstars.txt from ephe/ subdirectory
configure_file(
  ${swisseph_SOURCE_DIR}/ephe/sefstars.txt
  ${CMAKE_CURRENT_SOURCE_DIR}/sefstars.txt
  COPYONLY
)

# Copy ephemeris data files from ephe/ directory
set(SWEPH_EPHE_FILES
  seas_18.se1
  seasm18.se1
  semo_18.se1
  semom18.se1
  sepl_18.se1
  seplm18.se1
)

foreach(ephe_file ${SWEPH_EPHE_FILES})
  if(EXISTS ${swisseph_SOURCE_DIR}/ephe/${ephe_file})
    configure_file(
      ${swisseph_SOURCE_DIR}/ephe/${ephe_file}
      ${CMAKE_CURRENT_SOURCE_DIR}/${ephe_file}
      COPYONLY
    )
  else()
    message(WARNING "Ephemeris file not found: ${ephe_file}")
  endif()
endforeach()

message(STATUS "Swiss Ephemeris sources and data files copied successfully")

# Create a stamp file to indicate successful fetch
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/.swisseph_fetched "${SWISSEPH_VERSION}")
