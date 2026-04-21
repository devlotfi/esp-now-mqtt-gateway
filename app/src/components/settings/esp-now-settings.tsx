import {
  Button,
  Card,
  Label,
  ListBox,
  Select,
  Skeleton,
  toast,
} from "@heroui/react";
import { useContext, useState } from "react";
import { useTranslation } from "react-i18next";
import { useFormik } from "formik";
import { Save, InfoIcon, Eye, EyeOff, RefreshCw } from "lucide-react";
import ValidatedTextField from "../validated-text-field";
import * as yup from "yup";
import { $api } from "../../api/openapi-client";
import { useQueryClient } from "@tanstack/react-query";
import { AuthContext } from "../../context/auth-context";
import CardWithTitle from "../card-with-header";
import { AppContext } from "../../context/app-context";
import { isUnicastMac } from "../../validation/is-unicast-mac";
import { generateEspNowKey } from "../../utils/generate-esp-now-key";

export default function EspNowSettings() {
  const { t } = useTranslation();
  const { rebootModalState } = useContext(AppContext);
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");
  const queryClient = useQueryClient();

  const configQuery = $api.useQuery("get", "/api/esp-now", {
    baseUrl: authData.apiUrl,
    headers: {
      Authorization: `Bearer ${authData.token}`,
    },
  });

  const configMutation = $api.useMutation("post", "/api/esp-now", {
    onSuccess() {
      queryClient.resetQueries({
        queryKey: ["get", "/api/esp-now"],
      });
      rebootModalState.open();
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
    enableReinitialize: true,
    initialValues: {
      mac: configQuery.data?.mac || "",
      channel: configQuery.data?.channel || 1,
      pmk: configQuery.data?.pmk || "",
    },
    validationSchema: yup.object({
      mac: yup
        .string()
        .required("MAC is required")
        .transform((v) => v?.toUpperCase())
        .matches(/^([0-9A-F]{2}:){5}[0-9A-F]{2}$/, "Invalid MAC format")
        .test("espnow-mac", "Must be a valid ESP-NOW unicast MAC", (value) =>
          isUnicastMac(value),
        ),
      channel: yup.number().integer().min(1).max(13).required(),
      pmk: yup
        .string()
        .matches(/^[A-F0-9]{32}$/)
        .required(),
    }),
    onSubmit(values) {
      configMutation.mutate({
        baseUrl: authData.apiUrl,
        headers: {
          Authorization: `Bearer ${authData.token}`,
        },
        body: {
          mac: values.mac,
          channel: values.channel,
          pmk: values.pmk,
        },
      });
    },
  });

  const [isVisible, setIsVisible] = useState(false);
  const toggleVisibility = () => setIsVisible(!isVisible);

  if (configQuery.isLoading)
    return (
      <Card>
        <Card.Content className="gap-[1rem]">
          <Skeleton className="h-3 w-1/2 rounded-lg" />
          <Skeleton className="h-3 rounded-lg" />
          <Skeleton className="h-3 rounded-lg" />
          <Skeleton className="h-3 rounded-lg" />
        </Card.Content>
      </Card>
    );

  return (
    <CardWithTitle icon="router" title={"ESP-NOW"}>
      <form
        onSubmit={formik.handleSubmit}
        className="flex flex-col gap-[0.5rem] p-[1rem]"
      >
        <ValidatedTextField
          formik={formik}
          name="mac"
          textFieldProps={{
            isRequired: true,
          }}
          labelProps={{ children: "MAC" }}
        ></ValidatedTextField>
        <Select
          value={formik.values.channel}
          onChange={(value) => formik.setFieldValue("channel", value)}
          isRequired
        >
          <Label>{t("channel")}</Label>
          <Select.Trigger>
            <Select.Value />
            <Select.Indicator />
          </Select.Trigger>
          <Select.Popover>
            <ListBox>
              {[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13].map((channel) => (
                <ListBox.Item
                  key={channel}
                  id={channel}
                  textValue={`${channel}`}
                >
                  {channel}
                  <ListBox.ItemIndicator />
                </ListBox.Item>
              ))}
            </ListBox>
          </Select.Popover>
        </Select>
        <ValidatedTextField
          formik={formik}
          name="pmk"
          labelProps={{ children: "PMK" }}
          inputProps={{
            type: isVisible ? "text" : "password",
          }}
          textFieldProps={{ isRequired: true }}
          suffix={
            <div className="flex items-center gap-[0.1rem]">
              <Button
                isIconOnly
                variant="ghost"
                size="sm"
                onPress={toggleVisibility}
              >
                {isVisible ? <EyeOff></EyeOff> : <Eye></Eye>}
              </Button>
              <Button
                isIconOnly
                variant="ghost"
                size="sm"
                onPress={() => {
                  formik.setFieldValue("pmk", generateEspNowKey());
                }}
              >
                <RefreshCw></RefreshCw>
              </Button>
            </div>
          }
        ></ValidatedTextField>

        <Button
          fullWidth
          type="submit"
          isPending={configMutation.isPending}
          className="mt-[1rem]"
        >
          {t("save")}
          <Save></Save>
        </Button>
      </form>
    </CardWithTitle>
  );
}
