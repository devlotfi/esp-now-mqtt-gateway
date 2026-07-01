import { useTranslation } from "react-i18next";
import { Alert } from "@heroui/react";
import DataRow from "../data-row";
import type { paths } from "../../__generated__/schema";
import CardWithTitle from "../card-with-header";
import GrafanaLogo from "../../assets/grafana-logo.svg";

export default function MetricsCard({
  grafanaData,
}: {
  grafanaData: paths["/api/grafana"]["get"]["responses"]["200"]["content"]["application/json"];
}) {
  const { t } = useTranslation();

  return (
    <CardWithTitle icon="chart-line" title={t("metrics")}>
      <div className="flex px-[0.5rem] pt-[0.5rem]">
        <Alert className="p-[0.3rem] bg-surface">
          <Alert.Indicator>
            <img src={GrafanaLogo} alt="grafana" className="h-[2rem]" />
          </Alert.Indicator>
          <Alert.Content className="justify-center">
            <Alert.Title className="text-[12pt]">Grafana</Alert.Title>
          </Alert.Content>
        </Alert>
      </div>

      {!grafanaData.isSet ? (
        <div className="flex px-[0.5rem] pt-[0.5rem]">
          <Alert status="danger" className="px-[0.5rem] pt-[0.5rem]">
            <Alert.Indicator />
            <Alert.Content>
              <Alert.Title>{t("noMetricsConfig1")}</Alert.Title>
              <Alert.Description>{t("noMetricsConfig2")}</Alert.Description>
            </Alert.Content>
          </Alert>
        </div>
      ) : null}

      <div className="flex flex-col p-[1rem]">
        {grafanaData.isSet && grafanaData.config ? (
          <>
            <DataRow
              name={"URL"}
              value={grafanaData.config?.url}
              fold
            ></DataRow>
            <DataRow
              name={t("instanceId")}
              value={grafanaData.config?.instanceId}
              fold
            ></DataRow>
          </>
        ) : null}
      </div>
    </CardWithTitle>
  );
}
