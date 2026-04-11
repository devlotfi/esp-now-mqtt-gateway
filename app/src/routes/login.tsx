import { createFileRoute, Navigate } from "@tanstack/react-router";
import {
  Button,
  Card,
  Label,
  ListBox,
  Select,
  Tabs,
  toast,
} from "@heroui/react";
import {
  Computer,
  Download,
  Eye,
  EyeOff,
  InfoIcon,
  Moon,
  Settings,
  Sun,
  UserKey,
} from "lucide-react";
import { useContext, useState } from "react";
import { PWAContext } from "../context/pwa-context";
import LogoSVG from "../components/svg/LogoSVG";
import * as yup from "yup";
import { useFormik } from "formik";
import ValidatedTextField from "../components/validated-text-field";
import { useTranslation } from "react-i18next";
import { SectionTitle } from "../components/section-title";
import { ThemeContext } from "../context/theme-context";
import { ThemeOptions } from "../types/theme-options";
import { renderFlag } from "../utils/render-flag";
import { $api } from "../api/openapi-client";
import { AuthContext } from "../context/auth-context";
import { Constants } from "../constants";

export const Route = createFileRoute("/login")({
  component: RouteComponent,
});

function Navbar() {
  const { beforeInstallPromptEvent } = useContext(PWAContext);

  return (
    <div className="flex justify-between items-center min-h-[4rem] px-[0.7rem] bg-surface">
      <div className="flex items-center gap-[1rem] duration-300 transition-opacity">
        <LogoSVG className="h-[2.5rem] ml-[0.5rem]" />
        <div className="flex flex-col whitespace-nowrap leading-[1.2rem]">
          <div className="flex text-[12pt] md:text-[14pt] font-bold">
            ESP-NOW
          </div>
          <div className="flex text-[10pt] md:text-[12pt]">MQTT Gateway</div>
        </div>
      </div>

      <div className="flex gap-[1rem]">
        {beforeInstallPromptEvent ? (
          <Button
            isIconOnly
            variant="outline"
            className="size-[2.5rem] text-foreground bg-[color-mix(in_srgb,var(--surface),transparent_60%)]"
            onPress={() => beforeInstallPromptEvent.prompt()}
          >
            <Download className="size-[1.4rem]"></Download>
          </Button>
        ) : null}
      </div>
    </div>
  );
}

function SettingsTab() {
  const { themeOption, setTheme } = useContext(ThemeContext);
  const { t, i18n } = useTranslation();

  return (
    <>
      <SectionTitle icon="monitor-cog">{t("display")}</SectionTitle>

      <Select
        value={themeOption}
        onChange={(value) => setTheme(value?.toString() as ThemeOptions)}
      >
        <Label>{t("theme")}</Label>
        <Select.Trigger>
          <Select.Value />
          <Select.Indicator />
        </Select.Trigger>
        <Select.Popover>
          <ListBox>
            <ListBox.Item
              key={ThemeOptions.SYSTEM}
              id={ThemeOptions.SYSTEM}
              textValue={t("system")}
            >
              <div className="flex gap-[1rem] items-center">
                <div className="flex justify-center items-center h-[2rem] w-[2rem] bg-accent rounded-2xl">
                  <Computer className="text-accent-foreground"></Computer>
                </div>
                <div className="flex">{t("system")}</div>
              </div>
              <ListBox.ItemIndicator />
            </ListBox.Item>
            <ListBox.Item
              key={ThemeOptions.LIGHT}
              id={ThemeOptions.LIGHT}
              textValue={t("light")}
            >
              <div className="flex gap-[1rem] items-center">
                <div className="flex justify-center items-center h-[2rem] w-[2rem] bg-accent rounded-2xl">
                  <Sun className="text-accent-foreground"></Sun>
                </div>
                <div className="flex">{t("light")}</div>
              </div>
              <ListBox.ItemIndicator />
            </ListBox.Item>
            <ListBox.Item
              key={ThemeOptions.DARK}
              id={ThemeOptions.DARK}
              textValue={t("dark")}
            >
              <div className="flex gap-[1rem] items-center">
                <div className="flex justify-center items-center h-[2rem] w-[2rem] bg-accent rounded-2xl">
                  <Moon className="text-accent-foreground"></Moon>
                </div>
                <div className="flex">{t("dark")}</div>
              </div>
              <ListBox.ItemIndicator />
            </ListBox.Item>
          </ListBox>
        </Select.Popover>
      </Select>

      <Select
        value={i18n.language}
        onChange={(value) => i18n.changeLanguage(value?.toString())}
      >
        <Label>{t("language")}</Label>
        <Select.Trigger>
          <Select.Value />
          <Select.Indicator />
        </Select.Trigger>
        <Select.Popover>
          <ListBox>
            <ListBox.Item key={"ar"} id={"ar"} textValue={"العربية"}>
              <div className="flex gap-[1rem] items-center">
                <div className="flex justify-center items-center h-[2rem] w-[2rem] rounded-lg">
                  {renderFlag("ar")}
                </div>
                <div className="flex">العربية</div>
              </div>
              <ListBox.ItemIndicator />
            </ListBox.Item>
            <ListBox.Item key={"fr"} id={"fr"} textValue={"Français"}>
              <div className="flex gap-[1rem] items-center">
                <div className="flex justify-center items-center h-[2rem] w-[2rem] rounded-lg">
                  {renderFlag("fr")}
                </div>
                <div className="flex">Français</div>
              </div>
              <ListBox.ItemIndicator />
            </ListBox.Item>
            <ListBox.Item key={"en"} id={"en"} textValue={"English"}>
              <div className="flex gap-[1rem] items-center">
                <div className="flex justify-center items-center h-[2rem] w-[2rem] rounded-lg">
                  {renderFlag("en")}
                </div>
                <div className="flex">English</div>
              </div>
              <ListBox.ItemIndicator />
            </ListBox.Item>
          </ListBox>
        </Select.Popover>
      </Select>
    </>
  );
}

function RouteComponent() {
  const { authData, setAuthData } = useContext(AuthContext);
  const { t } = useTranslation();

  const { mutate, isPending } = $api.useMutation("post", "/login", {
    onSuccess(data) {
      if (data.token) {
        localStorage.setItem(
          Constants.API_URL_STORAGE_KEY,
          formik.values.apiUrl,
        );
        setAuthData({
          token: data.token,
          apiUrl: formik.values.apiUrl,
        });
      }
    },
    onError(error) {
      console.error(error);
      toast(`${t("error")}`, {
        indicator: <InfoIcon />,
        variant: "danger",
      });
    },
  });

  const formik = useFormik({
    initialValues: {
      apiUrl: localStorage.getItem(Constants.API_URL_STORAGE_KEY) || "",
      password: "",
    },
    validationSchema: yup.object({
      apiUrl: yup.string().url().required(),
      password: yup.string().required(),
    }),
    onSubmit(values) {
      mutate({
        baseUrl: values.apiUrl,
        body: {
          password: values.password,
        },
      });
    },
  });

  const [isVisible, setIsVisible] = useState(false);
  const toggleVisibility = () => setIsVisible(!isVisible);

  if (authData) return <Navigate to="/dashboard"></Navigate>;

  return (
    <div className="flex flex-col flex-1 h-dvh w-dvw">
      <Navbar></Navbar>
      <div className="flex flex-1 px-[1rem] justify-center items-center h-[calc(100dvh-4rem)] bg-background rounded-t-4xl border-t border-border overflow-x-hidden overflow-y-auto">
        <Card className="w-full max-w-[28rem] md:p-[2rem]">
          <Tabs className="w-full max-w-md">
            <Tabs.ListContainer>
              <Tabs.List aria-label="Options">
                <Tabs.Tab id="login" className="gap-[0.5rem]">
                  <UserKey size="1.3rem"></UserKey>
                  {t("login")}
                  <Tabs.Indicator />
                </Tabs.Tab>
                <Tabs.Tab id="settings" className="gap-[0.5rem]">
                  <Settings size="1.3rem"></Settings>
                  {t("settings")}
                  <Tabs.Indicator />
                </Tabs.Tab>
              </Tabs.List>
            </Tabs.ListContainer>
            <Tabs.Panel id="login">
              <SectionTitle icon="user-key">{t("login")}</SectionTitle>
              <form
                onSubmit={formik.handleSubmit}
                className="flex flex-col gap-[1rem]"
              >
                <ValidatedTextField
                  formik={formik}
                  name="apiUrl"
                  textFieldProps={{ isRequired: true }}
                  labelProps={{ children: t("apiUrl") }}
                ></ValidatedTextField>
                <ValidatedTextField
                  formik={formik}
                  name="password"
                  labelProps={{ children: t("password") }}
                  inputProps={{
                    type: isVisible ? "text" : "password",
                  }}
                  textFieldProps={{ isRequired: true }}
                  suffix={
                    <Button
                      isIconOnly
                      variant="ghost"
                      size="sm"
                      onPress={toggleVisibility}
                    >
                      {isVisible ? <EyeOff></EyeOff> : <Eye></Eye>}
                    </Button>
                  }
                ></ValidatedTextField>

                <Button
                  fullWidth
                  type="submit"
                  isPending={isPending}
                  className="mt-[1rem]"
                >
                  <UserKey></UserKey>
                  {t("login")}
                </Button>
              </form>
            </Tabs.Panel>
            <Tabs.Panel id="settings" className="flex flex-col gap-[1rem]">
              <SettingsTab></SettingsTab>
            </Tabs.Panel>
          </Tabs>
        </Card>
      </div>
    </div>
  );
}
