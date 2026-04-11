import "./i18n";
import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import "./index.css";
import {
  createHashHistory,
  createRouter,
  RouterProvider,
} from "@tanstack/react-router";
import { routeTree } from "./routeTree.gen.ts";
import { QueryClient, QueryClientProvider } from "@tanstack/react-query";
import { ThemeProvider } from "./provider/theme-provider.tsx";
import { Toast } from "@heroui/react";
import PWAProvider from "./provider/pwa-provider.tsx";
import AppProvider from "./provider/app-provider.tsx";
import NotFound from "./components/not-found.tsx";
import AuthProvider from "./provider/auth-provider.tsx";

const history = createHashHistory();

const router = createRouter({
  routeTree,
  history,
  defaultNotFoundComponent: NotFound,
});

declare module "@tanstack/react-router" {
  interface Register {
    router: typeof router;
  }
}

const queryClient = new QueryClient();

createRoot(document.getElementById("root")!).render(
  <StrictMode>
    <ThemeProvider>
      <div className="flex flex-col md:flex-row min-h-dvh min-w-dvw max-h-dvh max-w-dvw overflow-hidden bg-surface">
        <QueryClientProvider client={queryClient}>
          <Toast.Provider placement="top"></Toast.Provider>
          <PWAProvider>
            <AuthProvider>
              <AppProvider>
                <RouterProvider router={router}></RouterProvider>
              </AppProvider>
            </AuthProvider>
          </PWAProvider>
        </QueryClientProvider>
      </div>
    </ThemeProvider>
  </StrictMode>,
);
