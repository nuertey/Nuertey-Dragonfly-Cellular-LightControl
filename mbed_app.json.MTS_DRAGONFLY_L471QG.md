{
    "config": {
        "sock-type": "TCP",
        "echo-server-hostname": {
            "help": "Echo server host name.",
            "value": "\"echo.mbedcloudtesting.com\""
        },
        "echo-server-port": {
            "help": "Echo server port number.",
            "value": 7
        },
        "trace-level": {
            "help": "Options are TRACE_LEVEL_ERROR,TRACE_LEVEL_WARN,TRACE_LEVEL_INFO,TRACE_LEVEL_DEBUG",
            "macro_name": "MBED_TRACE_MAX_LEVEL",
            "value": "TRACE_LEVEL_DEBUG"
        }
    },
    "macros": [
        "MBED_TICKLESS=1"
    ],
    "target_overrides": {
        "*": {
            "target.network-default-interface-type": "CELLULAR",
            "events.shared-dispatch-from-application": true,
            "mbed-trace.enable": false,
            "lwip.ipv4-enabled": true,
            "ppp.ipv4-enabled": true,
            "lwip.ipv6-enabled": true,
            "ppp.ipv6-enabled": true,
            "lwip.ethernet-enabled": false,
            "lwip.ppp-enabled": true,
            "lwip.tcp-enabled": true,
            "platform.stdio-convert-newlines": true,
            "platform.stdio-baud-rate": 115200,
            "platform.default-serial-baud-rate": 115200,
            "platform.stdio-buffered-serial": true,
            "cellular.debug-at": true,
            "cellular.use-apn-lookup": true,
            "cellular.radio-access-technology": 8,
            "nsapi.default-cellular-sim-pin": null,
            "nsapi.default-cellular-plmn": null,
            "nsapi.default-cellular-apn":  "\"vzwinternet\"",
            "nsapi.default-cellular-username": null,
            "nsapi.default-cellular-password": null
        },
        "DISCO_L496AG": {
            "target.macros_add": [
                "CELLULAR_DEVICE=STModCellular"
            ],
            "target.components_add": ["STMOD_CELLULAR"],
            "stmod_cellular.provide-default": "true"
        }
    }
}
