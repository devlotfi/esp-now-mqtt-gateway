import { useState, type PropsWithChildren } from "react";
import { AppContext, AppContextInitialValue } from "../context/app-context";
import {
  Button,
  Modal,
  useOverlayState,
  type UseOverlayStateReturn,
} from "@heroui/react";
import { Power } from "lucide-react";
import { useTranslation } from "react-i18next";

function RebootModal({ state }: { state: UseOverlayStateReturn }) {
  const { t } = useTranslation();

  return (
    <Modal>
      <Modal.Backdrop isOpen={state.isOpen} onOpenChange={state.setOpen}>
        <Modal.Container>
          <Modal.Dialog className="sm:max-w-[360px]">
            <Modal.CloseTrigger />
            <Modal.Header>
              <Modal.Icon className="bg-default text-foreground">
                <Power className="size-5" />
              </Modal.Icon>
              <Modal.Heading>{t("rebooting")}</Modal.Heading>
            </Modal.Header>
            <Modal.Body>
              <Modal.Heading>{t("waitForReboot")}</Modal.Heading>
            </Modal.Body>
            <Modal.Footer>
              <Button className="w-full" slot="close">
                {t("continue")}
              </Button>
            </Modal.Footer>
          </Modal.Dialog>
        </Modal.Container>
      </Modal.Backdrop>
    </Modal>
  );
}

export default function AppProvider({ children }: PropsWithChildren) {
  const [sidebarOpen, setSidebarOpen] = useState<boolean>(
    AppContextInitialValue.sidebarOpen,
  );
  const rebootModalState = useOverlayState();

  return (
    <AppContext.Provider
      value={{
        scrollRef: AppContextInitialValue.scrollRef,
        sidebarOpen,
        setSidebarOpen,
        rebootModalState,
      }}
    >
      <RebootModal state={rebootModalState}></RebootModal>
      {children}
    </AppContext.Provider>
  );
}
