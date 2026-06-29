set(SYNTROPIC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../syntropic)

set(SYNTROPIC_SRCS
    ${SYNTROPIC_DIR}/util/syn_ringbuf.c
    ${SYNTROPIC_DIR}/util/syn_crc.c
    ${SYNTROPIC_DIR}/util/syn_fmt.c
    ${SYNTROPIC_DIR}/util/syn_fsm.c
    ${SYNTROPIC_DIR}/util/syn_json_write.c
    ${SYNTROPIC_DIR}/util/syn_json_read.c
    ${SYNTROPIC_DIR}/cli/syn_cli.c
    ${SYNTROPIC_DIR}/log/syn_log.c
    ${SYNTROPIC_DIR}/net/syn_http.c
    ${SYNTROPIC_DIR}/net/syn_httpd.c
    ${SYNTROPIC_DIR}/net/syn_dns.c
    ${SYNTROPIC_DIR}/net/syn_websocket.c
    ${SYNTROPIC_DIR}/net/syn_mqtt.c
    ${SYNTROPIC_DIR}/net/syn_transport_tcp.c
    ${SYNTROPIC_DIR}/system/syn_fwupdate.c
    ${SYNTROPIC_DIR}/system/syn_fwboot.c
    ${SYNTROPIC_DIR}/system/syn_boot.c
    ${SYNTROPIC_DIR}/system/syn_errlog.c
    ${SYNTROPIC_DIR}/storage/syn_param.c
    ${SYNTROPIC_DIR}/sched/syn_sched.c
    ${SYNTROPIC_DIR}/sched/syn_timer.c
    ${SYNTROPIC_DIR}/display/syn_canvas.c
    ${SYNTROPIC_DIR}/ui/syn_imgui.c
)

idf_component_register(
    SRCS
        "main.c"
        "port_esp32.c"
        ${SYNTROPIC_SRCS}
    INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/../../.."
    REQUIRES
        driver esp_wifi esp_event nvs_flash esp_netif esp_http_client
        esp_timer console vfs spi_flash app_update
)
