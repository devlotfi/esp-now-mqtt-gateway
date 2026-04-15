import { createFileRoute } from "@tanstack/react-router";
import { useTranslation } from "react-i18next";
import SectionHeader from "../../components/section-header";
import DisplaySettings from "../../components/settings/display-settings";
import PMKSettings from "../../components/settings/pmk-settings";
import PasswordSettings from "../../components/settings/password-settings";
import NotificationsSettings from "../../components/settings/notifications-settings";

export const Route = createFileRoute("/dashboard/settings")({
  component: RouteComponent,
});

function RouteComponent() {
  const { t } = useTranslation();

  return (
    <div className="flex flex-1 flex-col items-center p-[1rem]">
      <div className="flex flex-col w-full max-w-screen-sm pb-[5rem]">
        <SectionHeader icon="settings">{t("settings")}</SectionHeader>

        <div className="flex flex-col gap-[2rem]">
          <DisplaySettings></DisplaySettings>
          <NotificationsSettings></NotificationsSettings>
          <PMKSettings></PMKSettings>
          <PasswordSettings></PasswordSettings>
        </div>
      </div>
    </div>
  );
}
