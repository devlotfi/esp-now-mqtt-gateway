import { Button, cn } from "@heroui/react";
import { ChevronsLeft, Download } from "lucide-react";
import { useContext } from "react";
import { AppContext } from "../context/app-context";
import { PWAContext } from "../context/pwa-context";
import LogoSVG from "./svg/LogoSVG";

export default function Navbar() {
  const { sidebarOpen, setSidebarOpen } = useContext(AppContext);
  const { beforeInstallPromptEvent } = useContext(PWAContext);

  return (
    <div className="flex md:hidden flex-1 justify-between items-center min-h-[4rem] px-[0.7rem] bg-surface">
      <div className="flex items-center gap-[1rem]">
        <Button
          isIconOnly
          variant="outline"
          className="hidden lg:flex"
          size="lg"
          onPress={() => setSidebarOpen(!sidebarOpen)}
        >
          <ChevronsLeft
            className={cn(
              "size-[1.5rem] duration-300",
              !sidebarOpen && "rotate-180",
            )}
          />
        </Button>

        <div
          className={cn(
            "flex items-center gap-[1rem] duration-300 transition-opacity",
            sidebarOpen && "lg:opacity-0",
          )}
        >
          <LogoSVG className="h-[2.5rem] ml-[0.5rem]" />
          <div className="flex flex-col whitespace-nowrap leading-[1.2rem]">
            <div className="flex text-[12pt] md:text-[14pt] font-bold">
              ESP-NOW
            </div>
            <div className="flex text-[10pt] md:text-[12pt]">MQTT Gateway</div>
          </div>
        </div>
      </div>

      <div className="flex gap-[1rem]">
        {beforeInstallPromptEvent ? (
          <Button
            isIconOnly
            variant="outline"
            className="size-[2.5rem] text-foreground bg-[color-mix(in_srgb,var(--surface),transparent_60%)]"
            onPress={() => beforeInstallPromptEvent.prompt()}
          >
            <Download className="size-[1.4rem]"></Download>
          </Button>
        ) : null}
      </div>
    </div>
  );
}
