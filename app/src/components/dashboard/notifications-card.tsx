import { useTranslation } from "react-i18next";
import { Alert } from "@heroui/react";
import DataRow from "../data-row";
import type { paths } from "../../__generated__/schema";
import CardWithTitle from "../card-with-header";
import PushNotifierLogo from "../../assets/push-notifier-logo.svg";

export default function NotificationsCard({
  notificationsData,
}: {
  notificationsData: paths["/api/notifications"]["get"]["responses"]["200"]["content"]["application/json"];
}) {
  const { t } = useTranslation();

  return (
    <CardWithTitle icon="bell" title={t("notifications")}>
      <div className="flex px-[0.5rem] pt-[0.5rem]">
        <Alert className="p-[0.3rem] bg-surface">
          <Alert.Indicator>
            <img
              src={PushNotifierLogo}
              alt="push-notifier"
              className="h-[2rem]"
            />
          </Alert.Indicator>
          <Alert.Content className="justify-center">
            <Alert.Title className="text-[12pt]">Push Notifier</Alert.Title>
          </Alert.Content>
        </Alert>
      </div>

      {!notificationsData.isSet ? (
        <div className="flex px-[0.5rem] pt-[0.5rem]">
          <Alert status="danger">
            <Alert.Indicator />
            <Alert.Content>
              <Alert.Title>{t("noNotificationsConfig1")}</Alert.Title>
              <Alert.Description>
                {t("noNotificationsConfig2")}
              </Alert.Description>
            </Alert.Content>
          </Alert>
        </div>
      ) : null}

      <div className="flex flex-col p-[1rem]">
        <DataRow
          name={"URL"}
          value={notificationsData.config?.apiUrl}
          fold
        ></DataRow>
      </div>
    </CardWithTitle>
  );
}
