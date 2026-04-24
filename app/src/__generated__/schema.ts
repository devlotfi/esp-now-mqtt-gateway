export interface paths {
    "/api/auth/login": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        get?: never;
        put?: never;
        /** Authenticate and obtain a JWT token */
        post: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody: {
                content: {
                    "application/json": components["schemas"]["LoginRequest"];
                };
            };
            responses: {
                /** @description Login successful */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["LoginResponse"];
                    };
                };
                /** @description Missing or invalid request body */
                400: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
                /** @description Wrong password */
                403: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
            };
        };
        delete?: never;
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
    "/api/auth/set-password": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        get?: never;
        put?: never;
        /** Change the device password */
        post: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody: {
                content: {
                    "application/json": components["schemas"]["SetPasswordRequest"];
                };
            };
            responses: {
                /** @description Password updated successfully */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
            };
        };
        delete?: never;
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
    "/api/device/reboot": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        get?: never;
        put?: never;
        /** Reboot the device */
        post: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody?: never;
            responses: {
                /** @description Reboot initiated */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
            };
        };
        delete?: never;
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
    "/api/device/status": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        /** Get device health / status information */
        get: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody?: never;
            responses: {
                /** @description Device status */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["DeviceStatusResponse"];
                    };
                };
            };
        };
        put?: never;
        post?: never;
        delete?: never;
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
    "/api/network": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        /** Get current network configuration */
        get: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody?: never;
            responses: {
                /** @description Network config */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["NetworkConfigResponse"];
                    };
                };
            };
        };
        put?: never;
        /** Set network configuration (triggers reboot) */
        post: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody: {
                content: {
                    "application/json": components["schemas"]["NetworkSetConfigRequest"];
                };
            };
            responses: {
                /** @description Config saved; device will reboot */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                /** @description Invalid or incomplete request */
                400: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
            };
        };
        delete?: never;
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
    "/api/mqtt": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        /** Get MQTT broker configuration */
        get: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody?: never;
            responses: {
                /** @description MQTT config */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["MqttConfigResponse"];
                    };
                };
            };
        };
        put?: never;
        /** Set MQTT broker configuration (triggers reboot) */
        post: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody: {
                content: {
                    "application/json": components["schemas"]["MqttSetConfigRequest"];
                };
            };
            responses: {
                /** @description Config saved; device will reboot */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                /** @description Invalid or incomplete request */
                400: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
            };
        };
        delete?: never;
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
    "/api/notifications": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        /** Get push-notification configuration */
        get: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody?: never;
            responses: {
                /** @description Notification config */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["NotificationsConfigResponse"];
                    };
                };
            };
        };
        put?: never;
        /** Set push-notification configuration */
        post: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody: {
                content: {
                    "application/json": components["schemas"]["NotificationsSetConfigRequest"];
                };
            };
            responses: {
                /** @description Config saved */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                /** @description Invalid or incomplete request */
                400: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
            };
        };
        delete?: never;
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
    "/api/notifications/test": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        get?: never;
        put?: never;
        /** Send a test notification */
        post: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody?: never;
            responses: {
                /** @description Test notification sent */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                /** @description Notifications not configured */
                404: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
            };
        };
        delete?: never;
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
    "/api/esp-now/peers": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        /** List all ESP-NOW peers */
        get: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody?: never;
            responses: {
                /** @description Peer list */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["PeersResponse"];
                    };
                };
            };
        };
        put?: never;
        /** Add an ESP-NOW peer */
        post: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody: {
                content: {
                    "application/json": components["schemas"]["AddPeerRequest"];
                };
            };
            responses: {
                /** @description Peer added */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                /** @description Invalid MAC or LMK format */
                400: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
                /** @description Peer with same name or MAC already exists */
                403: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
                /** @description Maximum peer count reached */
                500: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
            };
        };
        delete?: never;
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
    "/api/esp-now/peers/{id}": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        get?: never;
        put?: never;
        post?: never;
        /** Delete an ESP-NOW peer by UUID */
        delete: {
            parameters: {
                query?: never;
                header?: never;
                path: {
                    /** @example 550e8400-e29b-41d4-a716-446655440000 */
                    id: string;
                };
                cookie?: never;
            };
            requestBody?: never;
            responses: {
                /** @description Peer deleted */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                /** @description Peer not found */
                404: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
            };
        };
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
    "/api/esp-now/topics/{id}": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        /** List MQTT topics subscribed for a peer */
        get: {
            parameters: {
                query?: never;
                header?: never;
                path: {
                    id: string;
                };
                cookie?: never;
            };
            requestBody?: never;
            responses: {
                /** @description Topic list */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["TopicsResponse"];
                    };
                };
                /** @description Peer not found */
                404: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
            };
        };
        put?: never;
        /** Add an MQTT topic subscription for a peer */
        post: {
            parameters: {
                query?: never;
                header?: never;
                path: {
                    id: string;
                };
                cookie?: never;
            };
            requestBody: {
                content: {
                    "application/json": components["schemas"]["AddTopicRequest"];
                };
            };
            responses: {
                /** @description Topic added and subscribed */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                /** @description Missing topic field */
                400: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
                /** @description Topic already exists for this peer */
                403: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
                /** @description Peer not found */
                404: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
                /** @description Maximum topic count reached */
                500: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
            };
        };
        /** Remove an MQTT topic subscription from a peer */
        delete: {
            parameters: {
                query?: never;
                header?: never;
                path: {
                    id: string;
                };
                cookie?: never;
            };
            requestBody: {
                content: {
                    "application/json": components["schemas"]["DeleteTopicRequest"];
                };
            };
            responses: {
                /** @description Topic removed (and MQTT unsubscribed if no other peers use it) */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                /** @description Missing topic field */
                400: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
                /** @description Peer or topic not found */
                404: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
            };
        };
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
    "/api/esp-now": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        /** Get ESP-NOW configuration (MAC and channel) */
        get: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody?: never;
            responses: {
                /** @description ESP-NOW config */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["EspNowConfigResponse"];
                    };
                };
            };
        };
        put?: never;
        /** Set ESP-NOW configuration (MAC and channel, triggers reboot) */
        post: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody: {
                content: {
                    "application/json": components["schemas"]["EspNowSetConfigRequest"];
                };
            };
            responses: {
                /** @description Config saved; device will reboot */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                /** @description Invalid MAC or channel */
                400: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["Error"];
                    };
                };
            };
        };
        delete?: never;
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
}
export type webhooks = Record<string, never>;
export interface components {
    schemas: {
        Error: {
            /**
             * @example INVALID_REQUEST
             * @enum {string}
             */
            error: "INVALID_REQUEST" | "WRONG_PASSWORD" | "NOT_FOUND" | "EXISTS" | "MAX_PEERS_REACHED" | "MAX_TOPICS_REACHED" | "CANNOT_SET_PMK_TO_ESP_NOW";
        };
        LoginRequest: {
            /** @example mysecretpassword */
            password: string;
        };
        LoginResponse: {
            /**
             * @description JWT bearer token (max 255 chars)
             * @example eyJhbGciOi...
             */
            token: string;
        };
        SetPasswordRequest: {
            /** @example mynewpassword */
            password: string;
        };
        DeviceStatusResponse: {
            heap: {
                /** @example 200000 */
                freeHeap: number;
                /** @example 327680 */
                heapSize: number;
                /** @example 180000 */
                minFreeHeap: number;
                /** @example 131072 */
                maxAllocHeap: number;
            };
            psram: {
                /** @example 4000000 */
                freePsram: number;
                /** @example 4194304 */
                psramSize: number;
                /** @example 3900000 */
                minFreePsram: number;
                /** @example 2097152 */
                maxAllocPsram: number;
            };
            storage: {
                /** @example 4194304 */
                flashChipSize: number;
                /** @example 1900000 */
                freeSketchSpace: number;
            };
            ethernet: {
                /** @example 192.168.1.100 */
                localIP: string;
                /** @example AA:BB:CC:DD:EE:FF */
                macAddress: string;
                /** @example true */
                linkUp: boolean;
                /** @example 100 */
                linkSpeed: number;
                /** @example true */
                fullDuplex: boolean;
            };
            general: {
                /**
                 * @description Milliseconds since last boot
                 * @example 123456
                 */
                uptime: number;
                /**
                 * Format: float
                 * @description CPU temperature in °C; null if sensor read failed
                 * @example 42.5
                 */
                cpuTemp?: number | null;
                /** @example true */
                mqttConnected: boolean;
                /**
                 * @description Current time as "YYYY-MM-DD HH:MM:SS"; null if NTP not synced
                 * @example 2025-04-24 14:30:00
                 */
                time?: string | null;
            };
        };
        NetworkConfigResponse: {
            /**
             * @example STATIC
             * @enum {string}
             */
            ipAssignment: "DHCP" | "STATIC";
            /** @description Present but empty when ipAssignment is DHCP; populated when STATIC */
            staticConfig?: {
                /** @example 192.168.1.100 */
                ip?: string;
                /** @example 192.168.1.1 */
                gateway?: string;
                /** @example 255.255.255.0 */
                subnet?: string;
                /** @example 8.8.8.8 */
                dns?: string;
            };
        };
        NetworkSetConfigRequest: {
            /**
             * @example STATIC
             * @enum {string}
             */
            ipAssignment: "DHCP" | "STATIC";
            /** @description Required when ipAssignment is STATIC */
            staticConfig?: {
                /** @example 192.168.1.100 */
                ip: string;
                /** @example 192.168.1.1 */
                gateway: string;
                /** @example 255.255.255.0 */
                subnet: string;
                /** @example 8.8.8.8 */
                dns: string;
            };
        };
        MqttConfigResponse: {
            /** @example true */
            isSet: boolean;
            /** @description Present only when isSet is true */
            config?: {
                /** @example true */
                useAuth: boolean;
                /** @example esp32-gateway */
                clientId: string;
                /** @example mqttuser */
                username: string;
                /** @example mqttpass */
                password: string;
                /** @example mqtt://broker.example.com:1883 */
                url: string;
            };
        };
        MqttSetConfigRequest: {
            /** @example esp32-gateway */
            clientId: string;
            /**
             * @description Valid MQTT URL (mqtt:// or mqtts://)
             * @example mqtt://broker.example.com:1883
             */
            url: string;
            /** @example true */
            useAuth: boolean;
            /**
             * @description Required when useAuth is true
             * @example mqttuser
             */
            username?: string;
            /**
             * @description Required when useAuth is true
             * @example mqttpass
             */
            password?: string;
        };
        NotificationsConfigResponse: {
            /** @example true */
            isSet: boolean;
            /** @description Present only when isSet is true */
            config?: {
                /** @example https://notify.example.com/send */
                apiUrl: string;
                /** @example supersecret */
                apiSecret: string;
            };
        };
        NotificationsSetConfigRequest: {
            /**
             * @description Valid HTTP/HTTPS URL
             * @example https://notify.example.com/send
             */
            apiUrl: string;
            /** @example supersecret */
            apiSecret: string;
        };
        Peer: {
            /**
             * Format: uuid
             * @example 550e8400-e29b-41d4-a716-446655440000
             */
            id: string;
            /** @example sensor-node-1 */
            name: string;
            /** @example AA:BB:CC:DD:EE:01 */
            mac: string;
            /**
             * @description 32-character hex string of the 16-byte Local Master Key
             * @example aabbccddeeff00112233445566778899
             */
            lmk: string;
        };
        PeersResponse: {
            peers: components["schemas"]["Peer"][];
        };
        AddPeerRequest: {
            /**
             * @description Human-readable peer name; must be unique
             * @example sensor-node-1
             */
            name: string;
            /**
             * @description Unicast MAC address; must be unique
             * @example AA:BB:CC:DD:EE:01
             */
            mac: string;
            /**
             * @description 32-character hex string representing the 16-byte Local Master Key
             * @example aabbccddeeff00112233445566778899
             */
            lmk: string;
        };
        TopicsResponse: {
            /**
             * @example [
             *       "sensors/temperature",
             *       "sensors/humidity"
             *     ]
             */
            topicList: string[];
        };
        AddTopicRequest: {
            /**
             * @description MQTT topic string; must be unique per peer
             * @example sensors/temperature
             */
            topic: string;
        };
        DeleteTopicRequest: {
            /** @example sensors/temperature */
            topic: string;
        };
        EspNowConfigResponse: {
            /**
             * @description Unicast MAC address in colon-separated hex notation
             * @example AA:BB:CC:DD:EE:FF
             */
            mac: string;
            /** @example 6 */
            channel: number;
            /** @example true */
            pmkSet: boolean;
            /**
             * @description Present only when pmkSet is true; 32-char hex string of the 16-byte PMK
             * @example 0102030405060708090a0b0c0d0e0f10
             */
            pmk?: string;
        };
        EspNowSetConfigRequest: {
            /**
             * @description Unicast MAC address (colon-separated hex)
             * @example AA:BB:CC:DD:EE:FF
             */
            mac: string;
            /** @example 6 */
            channel: number;
            /**
             * @description 32-character hex string representing the 16-byte Primary Master Key
             * @example 0102030405060708090a0b0c0d0e0f10
             */
            pmk: string;
        };
    };
    responses: never;
    parameters: never;
    requestBodies: never;
    headers: never;
    pathItems: never;
}
export type $defs = Record<string, never>;
export type operations = Record<string, never>;
