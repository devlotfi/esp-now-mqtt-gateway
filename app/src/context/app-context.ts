import type { UseOverlayStateReturn } from "@heroui/react";
import { createContext, createRef, type RefObject } from "react";

interface AppContext {
  scrollRef: RefObject<HTMLDivElement | null>;
  sidebarOpen: boolean;
  setSidebarOpen: (value: boolean) => void;
  rebootModalState: UseOverlayStateReturn;
}

export const AppContextInitialValue: AppContext = {
  scrollRef: createRef(),
  sidebarOpen: true,
  setSidebarOpen() {},
  rebootModalState: {} as UseOverlayStateReturn,
};

export const AppContext = createContext(AppContextInitialValue);
