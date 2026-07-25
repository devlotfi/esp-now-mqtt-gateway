idf_component_register(
  SRCS 
    "src/Dhcp.cpp"
    "src/Dns.cpp"
    "src/Ethernet.cpp"
    "src/EthernetClient.cpp"
    "src/EthernetServer.cpp"
    "src/EthernetUdp.cpp"
    "src/socket.cpp"
  INCLUDE_DIRS "src"
  REQUIRES espressif__arduino-esp32
)
