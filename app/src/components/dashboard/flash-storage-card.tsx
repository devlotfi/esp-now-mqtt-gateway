import { useTranslation } from "react-i18next";
import { Label, Meter } from "@heroui/react";
import DataRow from "../data-row";
import type { paths } from "../../__generated__/schema";
import CardWithTitle from "../card-with-header";

export default function FlashStorageCard({
  statusData,
}: {
  statusData: paths["/api/device/status"]["get"]["responses"]["200"]["content"]["application/json"]["storage"];
}) {
  const { t } = useTranslation();

  return (
    <CardWithTitle icon="hard-drive" title={t("flashStorage")}>
      <div className="flex p-[1rem]">
        <Meter
          maxValue={statusData.flashChipSize}
          minValue={0}
          value={statusData.flashChipSize - statusData.freeSketchSpace}
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
          name={t("flashChipSize")}
          value={
            <div className="flex">
              {(statusData.flashChipSize / 1000).toFixed(0)} kb
            </div>
          }
        ></DataRow>
        <DataRow
          name={t("freeSketchSpace")}
          value={
            <div className="flex">
              {(statusData.freeSketchSpace / 1000).toFixed(0)} kb
            </div>
          }
        ></DataRow>
      </div>
    </CardWithTitle>
  );
}
