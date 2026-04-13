export interface paths {
    "/login": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        get?: never;
        put?: never;
        /** Login */
        post: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody: {
                content: {
                    "application/json": components["schemas"]["PasswordRequest"];
                };
            };
            responses: {
                /** @description Success */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["LoginResponse"];
                    };
                };
                /** @description Wrong Password */
                403: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        /**
                         * @example {
                         *       "error": "WRONG_PASSWORD"
                         *     }
                         */
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
    "/set-password": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        get?: never;
        put?: never;
        /** Update password */
        post: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody: {
                content: {
                    "application/json": components["schemas"]["PasswordRequest"];
                };
            };
            responses: {
                /** @description Updated */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                401: components["responses"]["UnauthorizedError"];
            };
        };
        delete?: never;
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
    "/set-pmk": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        get?: never;
        put?: never;
        /** Set PMK */
        post: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody: {
                content: {
                    "application/json": components["schemas"]["SetPmkRequest"];
                };
            };
            responses: {
                /** @description Set */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                /** @description ESP_NOW Error */
                500: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        /**
                         * @example {
                         *       "error": "CANNOT_SET_PMK_TO_ESP_NOW"
                         *     }
                         */
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
    "/peers": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        /** List peers */
        get: {
            parameters: {
                query?: never;
                header?: never;
                path?: never;
                cookie?: never;
            };
            requestBody?: never;
            responses: {
                /** @description List */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["PeersListResponse"];
                    };
                };
            };
        };
        put?: never;
        /** Add peer */
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
                /** @description Added */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                /** @description Duplicate */
                403: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        /**
                         * @example {
                         *       "error": "EXISTS"
                         *     }
                         */
                        "application/json": components["schemas"]["Error"];
                    };
                };
                /** @description Limit Reached */
                500: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        /**
                         * @example {
                         *       "error": "MAX_PEERS_REACHED"
                         *     }
                         */
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
    "/peers/{id}": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
        get?: never;
        put?: never;
        post?: never;
        delete: {
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
                /** @description Deleted */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                404: components["responses"]["NotFoundError"];
            };
        };
        options?: never;
        head?: never;
        patch?: never;
        trace?: never;
    };
    "/topics/{id}": {
        parameters: {
            query?: never;
            header?: never;
            path?: never;
            cookie?: never;
        };
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
                /** @description Topics list */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        "application/json": components["schemas"]["TopicListResponse"];
                    };
                };
                404: components["responses"]["NotFoundError"];
            };
        };
        put?: never;
        /** Add single topic */
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
                    "application/json": components["schemas"]["TopicItem"];
                };
            };
            responses: {
                /** @description Topic added */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                /** @description Topic exists */
                403: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        /**
                         * @example {
                         *       "error": "EXISTS"
                         *     }
                         */
                        "application/json": components["schemas"]["Error"];
                    };
                };
                404: components["responses"]["NotFoundError"];
                /** @description Max topics */
                500: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content: {
                        /**
                         * @example {
                         *       "error": "MAX_TOPICS_REACHED"
                         *     }
                         */
                        "application/json": components["schemas"]["Error"];
                    };
                };
            };
        };
        /** Delete single topic */
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
                    "application/json": components["schemas"]["TopicItem"];
                };
            };
            responses: {
                /** @description Topic deleted */
                200: {
                    headers: {
                        [name: string]: unknown;
                    };
                    content?: never;
                };
                404: components["responses"]["NotFoundError"];
            };
        };
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
            /** @example INVALID_REQUEST */
            error: string;
        };
        PasswordRequest: {
            password: string;
        };
        LoginResponse: {
            token: string;
        };
        SetPmkRequest: {
            pmk: string;
        };
        Peer: {
            /** Format: uuid */
            id: string;
            name: string;
            mac: string;
        };
        PeersListResponse: {
            peers: components["schemas"]["Peer"][];
        };
        AddPeerRequest: {
            name: string;
            mac: string;
            lmk: string;
        };
        TopicItem: {
            topic: string;
        };
        TopicListResponse: {
            topicList: string[];
        };
    };
    responses: {
        /** @description Unauthorized */
        UnauthorizedError: {
            headers: {
                [name: string]: unknown;
            };
            content: {
                /**
                 * @example {
                 *       "error": "UNAUTHORIZED"
                 *     }
                 */
                "application/json": components["schemas"]["Error"];
            };
        };
        /** @description Not Found */
        NotFoundError: {
            headers: {
                [name: string]: unknown;
            };
            content: {
                /**
                 * @example {
                 *       "error": "NOT_FOUND"
                 *     }
                 */
                "application/json": components["schemas"]["Error"];
            };
        };
    };
    parameters: never;
    requestBodies: never;
    headers: never;
    pathItems: never;
}
export type $defs = Record<string, never>;
export type operations = Record<string, never>;
