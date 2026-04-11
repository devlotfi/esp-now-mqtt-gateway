import { createFileRoute, Outlet } from "@tanstack/react-router";
import BottomTabs from "../../components/bottom-tabs";
import Navbar from "../../components/navbar";
import { AppContext } from "../../context/app-context";
import { useContext } from "react";
import Sidebar from "../../components/sidebar";

export const Route = createFileRoute("/dashboard")({
  component: RouteComponent,
});

function RouteComponent() {
  const { scrollRef } = useContext(AppContext);

  return (
    <div className="flex flex-1">
      <Sidebar></Sidebar>
      <div className="flex flex-1 flex-col min-w-0">
        <Navbar></Navbar>
        <div
          ref={scrollRef}
          className="flex h-[calc(100dvh-4rem-5.5rem)] md:h-[calc(100dvh)] min-w-0 overflow-x-hidden overflow-y-auto bg-background border md:border-b-0 md:border-r-0 border-border rounded-4xl md:rounded-br-none md:rounded-tr-none"
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
