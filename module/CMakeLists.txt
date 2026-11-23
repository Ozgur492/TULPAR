# TULPAR ns-3 module (GPL-2.0-only).
#
# The agent modules are separate static targets, so the build itself enforces the
# dependency rules of RSD Table 4.25 and of MD1, MD5, MD9, MD14 and MD19: a module
# can include another module's headers only if it links that module's target.
# tools/check_static.py checks the include graph against tools/deps_rules.json.
#
# The ns-3 library "tulpar" holds the simulation harness (M-SIM): scenario
# controller, ground-truth store and event log. No agent target links it.

set(TULPAR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

function(tulpar_static_target name)
  cmake_parse_arguments(T "" "" "SOURCES;INCLUDE;DEPS" ${ARGN})
  add_library(${name} STATIC ${T_SOURCES})
  set_target_properties(${name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
  target_include_directories(${name} PUBLIC ${T_INCLUDE})
  target_link_libraries(${name} PUBLIC ${T_DEPS})
endfunction()

# Shared types and the write-only event sink.
tulpar_static_target(tulpar_common
  SOURCES ${TULPAR_DIR}/common/tulpar-event.cc
          ${TULPAR_DIR}/common/tulpar-agent-module.cc
  INCLUDE ${TULPAR_DIR}/common
  DEPS ${libcore}
)

# M-CRY: every other module may use its services (Table 4.25, I6).
tulpar_static_target(tulpar_cry
  SOURCES ${TULPAR_DIR}/agent/cry/tulpar-cry.cc
  INCLUDE ${TULPAR_DIR}/agent/cry
  DEPS tulpar_common
)

tulpar_static_target(tulpar_com
  SOURCES ${TULPAR_DIR}/agent/com/tulpar-com.cc
          ${TULPAR_DIR}/agent/com/tulpar-hello-header.cc
  INCLUDE ${TULPAR_DIR}/agent/com
  DEPS tulpar_common tulpar_cry ${libnetwork} ${libinternet}
)

foreach(m del evs obs sec mis)
  tulpar_static_target(tulpar_${m}
    SOURCES ${TULPAR_DIR}/agent/${m}/tulpar-${m}.cc
    INCLUDE ${TULPAR_DIR}/agent/${m}
    DEPS tulpar_common tulpar_cry
  )
endforeach()

# M-LSC links no agent module and no record decoder (MD19).
tulpar_static_target(tulpar_lsc
  SOURCES ${TULPAR_DIR}/agent/lsc/tulpar-lsc.cc
  INCLUDE ${TULPAR_DIR}/agent/lsc
  DEPS tulpar_common
)

# Agent root: wires the eight modules of one vehicle together.
tulpar_static_target(tulpar_agent
  SOURCES ${TULPAR_DIR}/agent/root/tulpar-agent.cc
  INCLUDE ${TULPAR_DIR}/agent/root
  DEPS tulpar_common tulpar_cry tulpar_com tulpar_del tulpar_evs tulpar_obs
       tulpar_sec tulpar_mis tulpar_lsc ${libnetwork}
)

build_lib(
  LIBNAME tulpar
  SOURCE_FILES sim/tulpar-scenario.cc
               sim/tulpar-ground-truth.cc
               sim/tulpar-event-log.cc
  HEADER_FILES sim/tulpar-scenario.h
  LIBRARIES_TO_LINK ${libcore}
                    ${libnetwork}
                    ${libinternet}
                    ${libmobility}
                    ${libwifi}
  TEST_SOURCES test/tulpar-g0-test-suite.cc
)

# Linked privately: the agent targets stay inside libtulpar and out of the ns-3
# export set, and nothing outside the harness can link an agent module directly.
target_link_libraries(tulpar PRIVATE tulpar_agent)
