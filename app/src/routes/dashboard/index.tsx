import { createFileRoute } from "@tanstack/react-router";
import { useTranslation } from "react-i18next";
import { $api } from "../../api/openapi-client";
import { useContext } from "react";
import { AuthContext } from "../../context/auth-context";
import SectionHeader from "../../components/section-header";
import { Button } from "@heroui/react";
import { RotateCw } from "lucide-react";
import LoadingScreen from "../../components/loading-screen";
import ErrorScreen from "../../components/error-screen";
import DeviceCard from "../../components/dashboard/device-card";
import MqttCard from "../../components/dashboard/mqtt-card";
import EthernetCard from "../../components/dashboard/ethernet-card";
import NetworkCard from "../../components/dashboard/network-card";
import NotificationsCard from "../../components/dashboard/notifications-card";
import HeapCard from "../../components/dashboard/heap-card";
import PsramCard from "../../components/dashboard/psram-card";
import FlashStorageCard from "../../components/dashboard/flash-storage-card";
import EspNowCard from "../../components/dashboard/esp-now-card";

export const Route = createFileRoute("/dashboard/")({
  component: RouteComponent,
});

function RouteComponent() {
  const { t } = useTranslation();
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");

  const statusQuery = $api.useQuery(
    "get",
    "/api/device/status",
    {
      baseUrl: authData.apiUrl,
      headers: {
        Authorization: `Bearer ${authData.token}`,
      },
    },
    {
      refetchInterval: 5000,
    },
  );

  const mqttQuery = $api.useQuery(
    "get",
    "/api/mqtt",
    {
      baseUrl: authData.apiUrl,
      headers: {
        Authorization: `Bearer ${authData.token}`,
      },
    },
    {
      refetchInterval: 5000,
    },
  );

  const networkQuery = $api.useQuery(
    "get",
    "/api/network",
    {
      baseUrl: authData.apiUrl,
      headers: {
        Authorization: `Bearer ${authData.token}`,
      },
    },
    {
      refetchInterval: 5000,
    },
  );

  const notificationsQuery = $api.useQuery(
    "get",
    "/api/notifications",
    {
      baseUrl: authData.apiUrl,
      headers: {
        Authorization: `Bearer ${authData.token}`,
      },
    },
    {
      refetchInterval: 5000,
    },
  );

  const espNowQuery = $api.useQuery(
    "get",
    "/api/esp-now",
    {
      baseUrl: authData.apiUrl,
      headers: {
        Authorization: `Bearer ${authData.token}`,
      },
    },
    {
      refetchInterval: 5000,
    },
  );

  if (
    statusQuery.isLoading ||
    mqttQuery.isLoading ||
    networkQuery.isLoading ||
    notificationsQuery.isLoading ||
    espNowQuery.isLoading
  )
    return <LoadingScreen></LoadingScreen>;
  if (
    !statusQuery.data ||
    !mqttQuery.data ||
    !networkQuery.data ||
    !notificationsQuery.data ||
    !espNowQuery.data ||
    statusQuery.isError ||
    mqttQuery.isError ||
    networkQuery.isError ||
    notificationsQuery.isError ||
    espNowQuery.isError
  )
    return <ErrorScreen></ErrorScreen>;

  return (
    <div className="flex flex-1 flex-col">
      <div className="flex p-[1rem]">
        <div className="flex flex-1 items-center justify-between h-[3.5rem] px-[0.5rem] bg-surface border border-border rounded-3xl">
          <SectionHeader
            icon="gauge-circle"
            className="py-0 gap-[1rem] md:gap-[1rem]"
            iconWrapperProps={{
              className: "size-[2.5rem]",
            }}
            iconProps={{
              className: "size-[1.8rem]",
            }}
            labelProps={{
              className: "text-[15pt]",
            }}
          >
            {t("dashboard")}
          </SectionHeader>

          <Button
            isIconOnly
            variant="outline"
            size="lg"
            isPending={statusQuery.isLoading}
            className="bg-background"
            onPress={() => statusQuery.refetch()}
          >
            <RotateCw className="size-[1.5rem]"></RotateCw>
          </Button>
        </div>
      </div>

      <div className="flex flex-col flex-1 items-center h-[calc(100dvh-4.5rem)] overflow-x-hidden overflow-y-auto">
        <div className="flex flex-col lg:flex-row w-full max-w-screen-lg p-[1rem] gap-[1rem]">
          <div className="flex flex-col flex-1 gap-[1rem]">
            <DeviceCard data={statusQuery.data.general}></DeviceCard>
            <MqttCard
              statusData={statusQuery.data.general}
              mqttData={mqttQuery.data}
            ></MqttCard>
            <EthernetCard statusData={statusQuery.data.ethernet}></EthernetCard>
            <NetworkCard networkData={networkQuery.data}></NetworkCard>
          </div>
          <div className="flex flex-col flex-1 gap-[1rem] ">
            <EspNowCard espNowData={espNowQuery.data}></EspNowCard>
            <HeapCard statusData={statusQuery.data.heap}></HeapCard>
            <PsramCard statusData={statusQuery.data.psram}></PsramCard>
            <FlashStorageCard
              statusData={statusQuery.data.storage}
            ></FlashStorageCard>
            <NotificationsCard
              notificationsData={notificationsQuery.data}
            ></NotificationsCard>
          </div>
        </div>
      </div>
    </div>
  );
}
