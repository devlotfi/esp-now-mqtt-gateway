import { Card, Label, ListBox, Select } from "@heroui/react";
import { createFileRoute } from "@tanstack/react-router";
import { useContext } from "react";
import { useTranslation } from "react-i18next";
import { ThemeContext } from "../../context/theme-context";
import { ThemeOptions } from "../../types/theme-options";
import { Computer, Moon, Sun } from "lucide-react";
import SectionHeader from "../../components/section-header";
import { SectionTitle } from "../../components/section-title";
import { renderFlag } from "../../utils/render-flag";

export const Route = createFileRoute("/dashboard/settings")({
  component: RouteComponent,
});

function DisplaySettings() {
  const { themeOption, setTheme } = useContext(ThemeContext);
  const { t, i18n } = useTranslation();

  return (
    <Card>
      <Card.Content className="flex flex-col gap-[0.7rem]">
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
      </Card.Content>
    </Card>
  );
}

function RouteComponent() {
  const { t } = useTranslation();

  return (
    <div className="flex flex-1 flex-col items-center p-[1rem]">
      <div className="flex flex-col w-full max-w-screen-sm pb-[5rem]">
        <SectionHeader icon="settings">{t("settings")}</SectionHeader>

        <DisplaySettings></DisplaySettings>
      </div>
    </div>
  );
}
