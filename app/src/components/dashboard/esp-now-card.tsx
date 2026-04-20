import { useTranslation } from "react-i18next";
import DataRow from "../data-row";
import type { paths } from "../../__generated__/schema";
import CardWithTitle from "../card-with-header";

export default function EspNowCard({
  espNowData,
}: {
  espNowData: paths["/api/esp-now"]["get"]["responses"]["200"]["content"]["application/json"];
}) {
  const { t } = useTranslation();

  return (
    <CardWithTitle icon="router" title={"ESP-NOW"}>
      <div className="flex flex-col p-[1rem]">
        <DataRow name={"MAC"} value={espNowData.mac}></DataRow>
        <DataRow name={t("channel")} value={espNowData.channel}></DataRow>
      </div>
    </CardWithTitle>
  );
}
