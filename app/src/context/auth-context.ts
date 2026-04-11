import { createContext } from "react";

export interface AuthData {
  token: string;
  apiUrl: string;
}

interface AuthContext {
  authData: AuthData | null;
  setAuthData: (value: AuthData | null) => void;
}

export const AuthContextInitialValue: AuthContext = {
  authData: {
    token:
      "x0At7iH4ZGPn0bHaQct8vA==.3KV78ityr9HA5ZyXcXW6LGKB15hb8BVyeyAW8emXW+w=",
    apiUrl: "http://192.168.1.15/",
  },
  setAuthData() {},
};

export const AuthContext = createContext(AuthContextInitialValue);
