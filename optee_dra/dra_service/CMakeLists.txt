project (dra_service C)

set (SRC host/main.c)

add_executable (${PROJECT_NAME} ${SRC})

# Diretório onde está o pcap.h
set(PCAP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/host/include/libpcap1.10.4)

# Diretório onde está a biblioteca libpcap.a ou libpcap.so
set(PCAP_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../out-br/target/usr/lib)

target_include_directories(${PROJECT_NAME}
			   PRIVATE ${PCAP_INCLUDE_DIR})

target_link_directories(${PROJECT_NAME} PRIVATE ${PCAP_LIB_DIR})

target_link_libraries (${PROJECT_NAME} PRIVATE pcap)

install (TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
