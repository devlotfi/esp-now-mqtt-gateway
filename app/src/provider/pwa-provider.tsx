import { useEffect, useState, type PropsWithChildren } from "react";
import { PWAContext } from "../context/pwa-context";
import { useRegisterSW } from "virtual:pwa-register/react";
import type { BeforeInstallPromptEvent } from "../types/before-install-prompt-event";

export default function PWAProvider({ children }: PropsWithChildren) {
  useRegisterSW({
    immediate: true,
  });

  const [beforeInstallPromptEvent, setBeforeInstallPromptEvent] =
    useState<BeforeInstallPromptEvent | null>(null);

  useEffect(() => {
    const handler = (e: Event) => {
      const event = e as BeforeInstallPromptEvent;
      event.preventDefault();
      console.log(event);
      setBeforeInstallPromptEvent(event);
    };
    window.addEventListener("beforeinstallprompt", handler);
    return () => window.removeEventListener("beforeinstallprompt", handler);
  }, []);

  return (
    <PWAContext.Provider
      value={{
        beforeInstallPromptEvent,
      }}
    >
      {children}
    </PWAContext.Provider>
  );
}
