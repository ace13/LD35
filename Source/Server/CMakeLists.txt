SET(HEADERS
	Application.hpp
	ConnectionManager.hpp
)

SET(SOURCES
	Application.cpp
	ConnectionManager.cpp

	main.cpp
)

ADD_EXECUTABLE(Server ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(Server
	Core
)
