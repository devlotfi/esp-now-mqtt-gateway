import { createContext } from "react";
import type { BeforeInstallPromptEvent } from "../types/before-install-prompt-event";

interface PWAContext {
  beforeInstallPromptEvent: BeforeInstallPromptEvent | null;
}

export const PWAContextInitialValue: PWAContext = {
  beforeInstallPromptEvent: null,
};

export const PWAContext = createContext(PWAContextInitialValue);
