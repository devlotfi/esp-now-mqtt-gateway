import { useTranslation } from "react-i18next";
import DataRow from "../data-row";
import type { paths } from "../../__generated__/schema";
import CardWithTitle from "../card-with-header";

export default function NetworkCard({
  networkData,
}: {
  networkData: paths["/api/network"]["get"]["responses"]["200"]["content"]["application/json"];
}) {
  const { t } = useTranslation();

  return (
    <CardWithTitle icon="network" title={t("network")}>
      <div className="flex flex-col p-[1rem]">
        <DataRow
          name={t("ipAssignment")}
          value={networkData.ipAssignment}
        ></DataRow>

        {networkData.ipAssignment === "STATIC" ? (
          <>
            <DataRow name={"IP"} value={networkData.staticConfig?.ip}></DataRow>
            <DataRow
              name={"Gateway"}
              value={networkData.staticConfig?.gateway}
            ></DataRow>
            <DataRow
              name={"Subnet"}
              value={networkData.staticConfig?.subnet}
            ></DataRow>
            <DataRow
              name={"DNS"}
              value={networkData.staticConfig?.dns}
            ></DataRow>
          </>
        ) : null}
      </div>
    </CardWithTitle>
  );
}
