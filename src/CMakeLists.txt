set(SOURCES yuvit.cpp)

add_executable(yuvit ${SOURCES})
target_link_libraries(yuvit freeimage)
install(TARGETS yuvit
		RUNTIME DESTINATION bin
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
		)