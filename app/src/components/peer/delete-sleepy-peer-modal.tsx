import {
  Button,
  Modal,
  toast,
  type UseOverlayStateReturn,
} from "@heroui/react";
import { useQueryClient } from "@tanstack/react-query";
import { useTranslation } from "react-i18next";
import { InfoIcon, Trash } from "lucide-react";
import type { paths } from "../../__generated__/schema";
import { $api } from "../../api/openapi-client";
import { useContext } from "react";
import { AuthContext } from "../../context/auth-context";

interface DeleteSleepyPeerModalProps {
  state: UseOverlayStateReturn;
  sleepyPeer: paths["/api/sleepy-peers"]["get"]["responses"]["200"]["content"]["application/json"]["sleepyPeers"][number];
}

export default function DeleteSleepyPeerModal({
  state,
  sleepyPeer,
}: DeleteSleepyPeerModalProps) {
  const { t } = useTranslation();
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");
  const queryClient = useQueryClient();

  const { mutate, isPending } = $api.useMutation(
    "delete",
    "/api/sleepy-peers/{id}",
    {
      onError() {
        toast(t("error"), {
          indicator: <InfoIcon />,
          variant: "danger",
        });
      },
      onSuccess() {
        queryClient.resetQueries({
          queryKey: ["get", "/api/sleepy-peers"],
        });
        state.close();
      },
    },
  );

  return (
    <Modal.Backdrop
      variant="blur"
      isOpen={state.isOpen}
      onOpenChange={state.setOpen}
    >
      <Modal.Container placement="center">
        <Modal.Dialog className="sm:max-w-[360px]">
          <Modal.CloseTrigger />
          <Modal.Header>
            <Modal.Icon className="bg-danger-soft text-danger-soft-foreground">
              <Trash className="size-5" />
            </Modal.Icon>
            <Modal.Heading>{t("deleteSleepyPeer")}</Modal.Heading>
          </Modal.Header>
          <Modal.Body>
            <div className="flex">{t("deleteConfirmation")}</div>
          </Modal.Body>
          <Modal.Footer>
            <Button
              variant="outline"
              onPress={() => state.close()}
              className="bg-background"
            >
              {t("cancel")}
            </Button>
            <Button
              variant="danger"
              isPending={isPending}
              onPress={() =>
                mutate({
                  params: {
                    path: {
                      id: sleepyPeer.id,
                    },
                  },
                  baseUrl: authData.apiUrl,
                  headers: {
                    Authorization: `Bearer ${authData.token}`,
                  },
                })
              }
            >
              {t("delete")}
            </Button>
          </Modal.Footer>
        </Modal.Dialog>
      </Modal.Container>
    </Modal.Backdrop>
  );
}
