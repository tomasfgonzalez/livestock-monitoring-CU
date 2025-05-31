idf_component_register(
  SRCS
  "display/oled.c"
  "display/status.c"
  "lsu-management/LSUManager.cpp"
  "lsu-management/LSU.cpp"
  "lora/rylr998.c"
  "lora/cu_comms.cpp"
  "tasks/rx_channel.cpp"
  "tasks/process_requests.cpp"
  "tasks/server_connection.cpp"
  "wi-fi/wifi.c"
  "wi-fi/MQTTClient.cpp"
  "wi-fi/mqtt_api.cpp"
  "request_queue.cpp"
  "uart.c"
  "main.cpp"
  INCLUDE_DIRS
  "lsu-management"
  "lora"
  "tasks"
  "wi-fi"
  "."
)

set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)