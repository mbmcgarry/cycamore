# ------------------- Add all Concrete Agents ----------------------------

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

CONFIGURE_FILE(behavior_functions.h "${CMAKE_CURRENT_SOURCE_DIR}/behavior_functions.h" @ONLY)

SET(CUSTOM_HEADERS "behavior_functions.h")

USE_CYCLUS("cycamore" "batch_reactor")

USE_CYCLUS("cycamore" "enrichment_facility")

#USE_CYCLUS("cycamore" "inpro_reactor")

USE_CYCLUS("cycamore" "sink")

USE_CYCLUS("cycamore" "source")

USE_CYCLUS("cycamore" "deploy_inst")

USE_CYCLUS("cycamore" "manager_inst")

USE_CYCLUS("cycamore" "growth_region")

INSTALL_CYCLUS_MODULE("cycamore" "" "NONE")

SET(TestSource ${cycamore_TEST_CC} PARENT_SCOPE)

# install header files
FILE(GLOB h_files "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
INSTALL(FILES ${h_files} DESTINATION include/cycamore COMPONENT cycamore)
