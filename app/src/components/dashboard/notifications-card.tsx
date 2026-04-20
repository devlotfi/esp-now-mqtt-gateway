import { useTranslation } from "react-i18next";
import { Alert } from "@heroui/react";
import DataRow from "../data-row";
import type { paths } from "../../__generated__/schema";
import CardWithTitle from "../card-with-header";

export default function NotificationsCard({
  notificationsData,
}: {
  notificationsData: paths["/api/notifications"]["get"]["responses"]["200"]["content"]["application/json"];
}) {
  const { t } = useTranslation();

  return (
    <CardWithTitle icon="bell" title={t("notifications")}>
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
