# ------------------- Add all Concrete Agents ----------------------------

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

USE_CYCLUS("cycamore" "batch_reactor")

USE_CYCLUS("cycamore" "enrichment_facility")

#USE_CYCLUS("cycamore" "inpro_reactor")

USE_CYCLUS("cycamore" "sink")

USE_CYCLUS("cycamore" "source")

USE_CYCLUS("cycamore" "deploy_inst")

USE_CYCLUS("cycamore" "manager_inst")

USE_CYCLUS("cycamore" "growth_region")

# MBM Behavior Functions
SET(cycamore_CC ${cycamore_CC} behavior_functions.cc)
SET(cycamore_H ${cycamore_H} behavior_functions.h)
SET(cycamore_TEST_CC ${cycamore_TEST_CC} "${CMAKE_CURRENT_SOURCE_DIR}/behavior_functions.cc")

INSTALL_CYCLUS_MODULE("cycamore" "" "NONE")

SET(TestSource ${cycamore_TEST_CC} PARENT_SCOPE)

# install header files
FILE(GLOB h_files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES ${h_files} DESTINATION include/cycamore COMPONENT cycamore)
