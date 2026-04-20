import { useTranslation } from "react-i18next";
import { Label, Meter } from "@heroui/react";
import DataRow from "../data-row";
import type { paths } from "../../__generated__/schema";
import CardWithTitle from "../card-with-header";

export default function HeapCard({
  statusData,
}: {
  statusData: paths["/api/device/status"]["get"]["responses"]["200"]["content"]["application/json"]["heap"];
}) {
  const { t } = useTranslation();

  return (
    <CardWithTitle icon="memory-stick" title={t("heap")}>
      <div className="flex p-[1rem]">
        <Meter
          maxValue={statusData.heapSize}
          minValue={0}
          value={statusData.heapSize - statusData.freeHeap}
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
          name={t("heapSize")}
          value={
            <div className="flex">
              {(statusData.heapSize / 1000).toFixed(0)} kb
            </div>
          }
        ></DataRow>
        <DataRow
          name={t("freeHeap")}
          value={
            <div className="flex">
              {(statusData.freeHeap / 1000).toFixed(0)} kb
            </div>
          }
        ></DataRow>
        <DataRow
          name={t("minFreeHeap")}
          value={
            <div className="flex">
              {(statusData.minFreeHeap / 1000).toFixed(0)} kb
            </div>
          }
        ></DataRow>
        <DataRow
          name={t("maxAllocHeap")}
          value={
            <div className="flex">
              {(statusData.maxAllocHeap / 1000).toFixed(0)} kb
            </div>
          }
        ></DataRow>
      </div>
    </CardWithTitle>
  );
}
