import { useTranslation } from "react-i18next";
import { Label, Meter } from "@heroui/react";
import DataRow from "../data-row";
import type { paths } from "../../__generated__/schema";
import CardWithTitle from "../card-with-header";

export default function PsramCard({
  statusData,
}: {
  statusData: paths["/api/device/status"]["get"]["responses"]["200"]["content"]["application/json"]["psram"];
}) {
  const { t } = useTranslation();

  return (
    <CardWithTitle icon="memory-stick" title={t("PSRAM")}>
      <div className="flex p-[1rem]">
        <Meter
          maxValue={statusData.psramSize}
          minValue={0}
          value={statusData.psramSize - statusData.freePsram}
        >
          <Label>{t("usage")}</Label>
          <Meter.Output />
          <Meter.Track>
            <Meter.Fill />
          </Meter.Track>
        </Meter>
      </div>

      <div className="flex flex-col p-[1rem]">
        <DataRow
          name={t("PSRAMSize")}
          value={
            <div className="flex">
              {(statusData.psramSize / 1000).toFixed(0)} kb
            </div>
          }
        ></DataRow>
        <DataRow
          name={t("freePSRAM")}
          value={
            <div className="flex">
              {(statusData.freePsram / 1000).toFixed(0)} kb
            </div>
          }
        ></DataRow>
        <DataRow
          name={t("minFreePSRAM")}
          value={
            <div className="flex">
              {(statusData.minFreePsram / 1000).toFixed(0)} kb
            </div>
          }
        ></DataRow>
        <DataRow
          name={t("maxAllocPSRAM")}
          value={
            <div className="flex">
              {(statusData.maxAllocPsram / 1000).toFixed(0)} kb
            </div>
          }
        ></DataRow>
      </div>
    </CardWithTitle>
  );
}
