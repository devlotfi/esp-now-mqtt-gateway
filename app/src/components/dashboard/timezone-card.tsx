import { useTranslation } from "react-i18next";
import DataRow from "../data-row";
import type { paths } from "../../__generated__/schema";
import CardWithTitle from "../card-with-header";

export default function TimezoneCard({
  timezoneData,
}: {
  timezoneData: paths["/api/timezone"]["get"]["responses"]["200"]["content"]["application/json"];
}) {
  const { t } = useTranslation();

  return (
    <CardWithTitle icon="clock" title={t("timezone")}>
      <div className="flex flex-col p-[1rem]">
        {timezoneData.config.iana ? (
          <DataRow name={"IANA"} value={timezoneData.config?.iana}></DataRow>
        ) : null}
        <DataRow
          name={"POSIX TZ"}
          value={timezoneData.config?.timezonePosix}
        ></DataRow>
      </div>
    </CardWithTitle>
  );
}
