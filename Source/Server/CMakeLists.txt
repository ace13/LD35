SET(HEADERS
	Application.hpp
)

SET(SOURCES
	Application.cpp

	main.cpp
)

ADD_EXECUTABLE(Server ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(Server
	Core
)
