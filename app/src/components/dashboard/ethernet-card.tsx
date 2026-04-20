import { useTranslation } from "react-i18next";
import DataRow from "../data-row";
import type { paths } from "../../__generated__/schema";
import { cn } from "@heroui/styles";
import CardWithTitle from "../card-with-header";
import { Chip } from "@heroui/react";
import { Cable, Unplug } from "lucide-react";

export default function EthernetCard({
  statusData,
}: {
  statusData: paths["/api/device/status"]["get"]["responses"]["200"]["content"]["application/json"]["ethernet"];
}) {
  const { t } = useTranslation();

  return (
    <CardWithTitle icon="ethernet-port" title="Ethernet">
      <div className="flex flex-col p-[1rem]">
        <div className="flex pb-[1rem]">
          {statusData.linkUp ? (
            <Chip
              color="success"
              variant="primary"
              size="lg"
              className="pl-[0.3rem] gap-[0.5rem]"
            >
              <Cable className="size-[1.5rem]"></Cable>
              {t("connected")}
            </Chip>
          ) : (
            <Chip
              color="danger"
              variant="primary"
              size="lg"
              className="pl-[0.3rem] gap-[0.5rem]"
            >
              <Unplug className="size-[1.5rem]"></Unplug>
              {t("disconnected")}
            </Chip>
          )}
        </div>

        <DataRow name={"MAC"} value={statusData.macAddress}></DataRow>
        <DataRow name={"IP"} value={statusData.localIP}></DataRow>
        <DataRow
          name={t("linkSpeed")}
          value={<div className="flex">{statusData.linkSpeed} mbps</div>}
        ></DataRow>
        <DataRow
          name={"Full-Duplex"}
          value={
            <div
              className={cn(
                statusData.fullDuplex ? "text-success" : "text-danger",
              )}
            >
              {JSON.stringify(statusData.fullDuplex)}
            </div>
          }
        ></DataRow>
      </div>
    </CardWithTitle>
  );
}
