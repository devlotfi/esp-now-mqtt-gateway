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
import AddSleepyPeerModal from "../../components/peer/add-sleepy-peer-modal";
import SleepyPeerComponent from "../../components/peer/sleepy-peer-component";

export const Route = createFileRoute("/dashboard/peers")({
  component: RouteComponent,
});

function RouteComponent() {
  const { t } = useTranslation();
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");

  const peersQuery = $api.useQuery("get", "/api/peers", {
    baseUrl: authData.apiUrl,
    headers: {
      Authorization: `Bearer ${authData.token}`,
    },
  });

  const sleepyPeersQuery = $api.useQuery("get", "/api/sleepy-peers", {
    baseUrl: authData.apiUrl,
    headers: {
      Authorization: `Bearer ${authData.token}`,
    },
  });

  const addPeerModalState = useOverlayState();
  const addSleepyPeerModalState = useOverlayState();

  return (
    <>
      <AddPeerModal state={addPeerModalState}></AddPeerModal>
      <AddSleepyPeerModal state={addSleepyPeerModalState}></AddSleepyPeerModal>

      <div className="flex flex-1 flex-col items-center md:px-[1rem]">
        <div className="flex flex-1 flex-col max-w-screen-lg w-full pb-[5rem]">
          <div className="flex justify-between items-center z-10 pt-[1rem] gap-[1rem] px-[2rem]">
            <SectionHeader icon="microchip">{t("peers")}</SectionHeader>
            <div className="hidden md:flex flex-1 h-[1px] bg-separator"></div>
            <Button
              isIconOnly
              variant="outline"
              className="size-[3rem] text-foreground bg-surface"
              onPress={() => addPeerModalState.open()}
            >
              <Plus className="size-[2rem]"></Plus>
            </Button>
          </div>

          {peersQuery.isLoading ? (
            <LoadingScreen></LoadingScreen>
          ) : peersQuery.isError ? (
            <ErrorScreen></ErrorScreen>
          ) : peersQuery.data?.peers && peersQuery.data.peers.length ? (
            <div className="grid grid-cols-1 lg:grid-cols-2 gap-[1rem] p-[1rem]">
              {peersQuery.data.peers.map((peer) => (
                <PeerComponent key={peer.id} peer={peer}></PeerComponent>
              ))}
            </div>
          ) : (
            <div className="flex flex-1 text-center justify-center items-center flex-col gap-[1rem] px-[0.5rem]">
              <EmptySVG className="h-[10rem]" />
              <div className="flex text-foreground text-[18pt] font-bold">
                {t("emptyList")}...
              </div>
            </div>
          )}

          <div className="flex justify-between items-center z-10 gap-[1rem] px-[2rem]">
            <SectionHeader icon="moon">{t("sleepyPeers")}</SectionHeader>
            <div className="hidden md:flex flex-1 h-[1px] bg-separator"></div>
            <Button
              isIconOnly
              variant="outline"
              className="size-[3rem] text-foreground bg-surface"
              onPress={() => addSleepyPeerModalState.open()}
            >
              <Plus className="size-[2rem]"></Plus>
            </Button>
          </div>

          {sleepyPeersQuery.isLoading ? (
            <LoadingScreen></LoadingScreen>
          ) : sleepyPeersQuery.isError ? (
            <ErrorScreen></ErrorScreen>
          ) : sleepyPeersQuery.data?.sleepyPeers &&
            sleepyPeersQuery.data.sleepyPeers.length ? (
            <div className="grid grid-cols-1 lg:grid-cols-2 gap-[1rem] p-[1rem]">
              {sleepyPeersQuery.data.sleepyPeers.map((sleepyPeer) => (
                <SleepyPeerComponent
                  key={sleepyPeer.id}
                  sleepyPeer={sleepyPeer}
                ></SleepyPeerComponent>
              ))}
            </div>
          ) : (
            <div className="flex flex-1 text-center justify-center items-center flex-col gap-[1rem] px-[0.5rem]">
              <EmptySVG className="h-[10rem]" />
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
