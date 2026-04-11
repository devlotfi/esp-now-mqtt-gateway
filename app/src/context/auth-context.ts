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
  authData: null,
  setAuthData() {},
};

export const AuthContext = createContext(AuthContextInitialValue);
