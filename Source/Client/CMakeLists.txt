SET(HEADERS
	Application.hpp
	ConnectionManager.hpp
	../Shared/NetworkedObject.hpp
	ResourceManager.hpp
	ResourceManager.inl
	ServerContainer.hpp
	
	IState.hpp
	StateManager.hpp

	States/GameState.hpp
	States/IntroState.hpp
	States/MenuState.hpp
)

SET(SOURCES
	Application.cpp
	ConnectionManager.cpp
	../Shared/NetworkedObject.cpp
	ResourceManager.cpp
	ServerContainer.cpp
	
	StateManager.cpp

	States/GameState.cpp
	States/IntroState.cpp
	States/MenuState.cpp

	main.cpp
)

SOURCE_GROUP("Header files\\States" REGULAR_EXPRESSION "States/[^./]+.(hpp|inl)")
SOURCE_GROUP("Source files\\States" REGULAR_EXPRESSION "States/[^./]+.cpp")

SOURCE_GROUP("Header files\\" REGULAR_EXPRESSION "[^./]+.(hpp|inl)")
SOURCE_GROUP("Source files\\" REGULAR_EXPRESSION "[^./]+.cpp")

ADD_DEFINITIONS(-DLD35_CLIENT)

ADD_EXECUTABLE(Client ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(Client
	Core
)
