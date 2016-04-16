SET(HEADERS
	Application.hpp
	ConnectionManager.hpp
	../Shared/NetworkedObject.hpp
)

SET(SOURCES
	Application.cpp
	ConnectionManager.cpp
	../Shared/NetworkedObject.cpp

	main.cpp
)

ADD_DEFINITIONS(-DLD35_SERVER)
ADD_EXECUTABLE(Server ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(Server
	Core
)
