import { useState, type PropsWithChildren } from "react";
import {
  AuthContext,
  AuthContextInitialValue,
  type AuthData,
} from "../context/auth-context";

export default function AuthProvider({ children }: PropsWithChildren) {
  const [authData, setAuthData] = useState<AuthData | null>(
    AuthContextInitialValue.authData,
  );

  return (
    <AuthContext.Provider
      value={{
        authData,
        setAuthData,
      }}
    >
      {children}
    </AuthContext.Provider>
  );
}
