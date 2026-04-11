import { createFileRoute, Navigate, Outlet } from "@tanstack/react-router";
import BottomTabs from "../../components/bottom-tabs";
import Navbar from "../../components/navbar";
import { AppContext } from "../../context/app-context";
import { useContext } from "react";
import Sidebar from "../../components/sidebar";
import { AuthContext } from "../../context/auth-context";

export const Route = createFileRoute("/dashboard")({
  component: RouteComponent,
});

function RouteComponent() {
  const { authData } = useContext(AuthContext);
  const { scrollRef } = useContext(AppContext);

  if (!authData) return <Navigate to="/login"></Navigate>;

  return (
    <div className="flex flex-1">
      <Sidebar></Sidebar>
      <div className="flex flex-1 flex-col md:p-[1rem] md:pl-0 md:pt-0 min-w-0">
        <Navbar></Navbar>
        <div
          ref={scrollRef}
          className="flex h-[calc(100dvh-4rem-5.5rem)] md:h-[calc(100dvh-1rem-4rem)] min-w-0 overflow-x-hidden overflow-y-auto bg-background border border-border rounded-4xl"
          style={{
            scrollbarColor:
              "color-mix(in srgb, var(--accent), transparent 30%) color-mix(in srgb, var(--surface), transparent 80%)",
            scrollbarWidth: "thin",
          }}
        >
          <Outlet />
        </div>
        <BottomTabs></BottomTabs>
      </div>
    </div>
  );
}
