import {
  Button,
  Card,
  Label,
  ListBox,
  Select,
  Skeleton,
  toast,
} from "@heroui/react";
import { createFileRoute } from "@tanstack/react-router";
import { useContext, useState } from "react";
import { useTranslation } from "react-i18next";
import { ThemeContext } from "../../context/theme-context";
import { ThemeOptions } from "../../types/theme-options";
import {
  Check,
  Computer,
  Eye,
  EyeOff,
  InfoIcon,
  Moon,
  RefreshCw,
  Save,
  Sun,
} from "lucide-react";
import SectionHeader from "../../components/section-header";
import { SectionTitle } from "../../components/section-title";
import { renderFlag } from "../../utils/render-flag";
import { useFormik } from "formik";
import * as yup from "yup";
import { $api } from "../../api/openapi-client";
import ValidatedTextField from "../../components/validated-text-field";
import { AuthContext } from "../../context/auth-context";
import { generateEspNowKey } from "../../utils/generate-esp-now-key";
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
