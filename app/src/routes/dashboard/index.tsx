import { createFileRoute } from "@tanstack/react-router";
import { useTranslation } from "react-i18next";
import SectionHeader from "../../components/section-header";
import { Button, useOverlayState } from "@heroui/react";
import EmptySVG from "../../components/svg/EmptySVG";
import { Plus } from "lucide-react";
import { $api } from "../../api/openapi-client";
import { useContext } from "react";
import { AuthContext } from "../../context/auth-context";
import LoadingScreen from "../../components/loading-screen";
import ErrorScreen from "../../components/error-screen";
import PeerComponent from "../../components/peer/peer-component";
import AddPeerModal from "../../components/peer/add-peer-modal";

export const Route = createFileRoute("/dashboard/")({
  component: RouteComponent,
});

function RouteComponent() {
  const { t } = useTranslation();
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");

  const { data, isLoading, isError } = $api.useQuery("get", "/peers", {
    baseUrl: authData.apiUrl,
    headers: {
      Authorization: `Bearer ${authData.token}`,
    },
  });

  const addPeerModalState = useOverlayState();

  return (
    <>
      <AddPeerModal state={addPeerModalState}></AddPeerModal>

      <div className="flex flex-1 flex-col items-center">
        <div className="flex flex-1 flex-col max-w-screen-lg w-full">
          <div className="flex justify-between items-center z-10 py-[1rem] px-[2rem]">
            <SectionHeader icon="microchip">{t("peers")}</SectionHeader>

            <Button
              isIconOnly
              variant="outline"
              className="size-[3rem] text-foreground bg-surface"
              onPress={() => addPeerModalState.open()}
            >
              <Plus className="size-[2rem]"></Plus>
            </Button>
          </div>

          {isLoading ? (
            <LoadingScreen></LoadingScreen>
          ) : isError ? (
            <ErrorScreen></ErrorScreen>
          ) : data?.peers && data.peers.length ? (
            <div className="grid grid-cols-1 lg:grid-cols-2 gap-[1rem] p-[1rem] pb-[5rem]">
              {data.peers.map((peer) => (
                <PeerComponent key={peer.id} peer={peer}></PeerComponent>
              ))}
            </div>
          ) : (
            <div className="flex flex-1 text-center justify-center items-center flex-col gap-[1rem] px-[0.5rem]">
              <EmptySVG className="h-[12rem] md:h-[15rem]" />
              <div className="flex text-foreground text-[18pt] font-bold">
                {t("emptyList")}...
              </div>
            </div>
          )}
        </div>
      </div>
    </>
  );
}
