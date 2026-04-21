import { createFileRoute } from "@tanstack/react-router";
import { useTranslation } from "react-i18next";
import SectionHeader from "../../components/section-header";
import DisplaySettings from "../../components/settings/display-settings";
import PasswordSettings from "../../components/settings/password-settings";
import NotificationsSettings from "../../components/settings/notifications-settings";
import MqttSettings from "../../components/settings/mqtt-settings";
import NetworkSettings from "../../components/settings/network-settings";
import EspNowSettings from "../../components/settings/esp-now-settings";

export const Route = createFileRoute("/dashboard/settings")({
  component: RouteComponent,
});

function RouteComponent() {
  const { t } = useTranslation();

  return (
    <div className="flex flex-1 flex-col items-center p-[1rem]">
      <div className="flex flex-col w-full max-w-screen-lg pb-[5rem]">
        <SectionHeader icon="settings">{t("settings")}</SectionHeader>

        <div className="flex flex-col lg:flex-row gap-[1rem]">
          <div className="flex flex-1 flex-col gap-[1rem]">
            <DisplaySettings></DisplaySettings>
            <MqttSettings></MqttSettings>
            <NotificationsSettings></NotificationsSettings>
          </div>
          <div className="flex flex-1 flex-col gap-[1rem]">
            <NetworkSettings></NetworkSettings>
            <EspNowSettings></EspNowSettings>
            <PasswordSettings></PasswordSettings>
          </div>
        </div>
      </div>
    </div>
  );
}
