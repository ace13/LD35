SET(HEADERS
	Application.hpp
	ResourceManager.hpp
	ResourceManager.inl
	ServerContainer.hpp
)

SET(SOURCES
	Application.cpp
	ResourceManager.cpp
	ServerContainer.cpp

	main.cpp
)

ADD_EXECUTABLE(Client ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(Client
	Core
)
