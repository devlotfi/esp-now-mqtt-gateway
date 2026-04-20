import { useTranslation } from "react-i18next";
import { Alert, Chip } from "@heroui/react";
import DataRow from "../data-row";
import type { paths } from "../../__generated__/schema";
import CardWithTitle from "../card-with-header";
import { Cable, Unplug } from "lucide-react";

export default function MqttCard({
  statusData,
  mqttData,
}: {
  statusData: paths["/api/device/status"]["get"]["responses"]["200"]["content"]["application/json"]["general"];
  mqttData: paths["/api/mqtt"]["get"]["responses"]["200"]["content"]["application/json"];
}) {
  const { t } = useTranslation();

  return (
    <CardWithTitle icon="chevrons-left-right-ellipsis" title="MQTT">
      {!mqttData.isSet ? (
        <div className="flex px-[0.5rem] pt-[0.5rem]">
          <Alert status="danger" className="px-[0.5rem] pt-[0.5rem]">
            <Alert.Indicator />
            <Alert.Content>
              <Alert.Title>{t("noMqttConfig1")}</Alert.Title>
              <Alert.Description>{t("noMqttConfig2")}</Alert.Description>
            </Alert.Content>
          </Alert>
        </div>
      ) : null}

      <div className="flex flex-col p-[1rem]">
        <div className="flex pb-[1rem]">
          {statusData.mqttConnected ? (
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
        {mqttData.isSet && mqttData.config ? (
          <>
            <DataRow
              name={t("clientId")}
              value={mqttData.config?.clientId}
              fold
            ></DataRow>
            <DataRow name={"URL"} value={mqttData.config?.url} fold></DataRow>
            <DataRow
              name={t("username")}
              value={mqttData.config?.username}
              fold
            ></DataRow>
          </>
        ) : null}
      </div>
    </CardWithTitle>
  );
}
