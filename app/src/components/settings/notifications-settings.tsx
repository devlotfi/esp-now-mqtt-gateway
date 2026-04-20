import { Alert, Button, Card, Separator, Skeleton, toast } from "@heroui/react";
import { useContext, useState } from "react";
import { useTranslation } from "react-i18next";
import { useFormik } from "formik";
import { EyeOff, Eye, Save, Check, InfoIcon, BellRing } from "lucide-react";
import ValidatedTextField from "../validated-text-field";
import * as yup from "yup";
import { $api } from "../../api/openapi-client";
import { useQueryClient } from "@tanstack/react-query";
import { AuthContext } from "../../context/auth-context";
import CardWithTitle from "../card-with-header";

export default function NotificationsSettings() {
  const { t } = useTranslation();
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");
  const queryClient = useQueryClient();

  const configQuery = $api.useQuery("get", "/api/notifications", {
    baseUrl: authData.apiUrl,
    headers: {
      Authorization: `Bearer ${authData.token}`,
    },
  });

  const configMutation = $api.useMutation("post", "/api/notifications", {
    onSuccess() {
      queryClient.resetQueries({
        queryKey: ["get", "/api/notifications"],
      });
      toast(t("actionSuccess"), {
        indicator: <Check />,
        variant: "success",
      });
    },
    onError(error) {
      console.error(error);
      toast(`${t("error")}`, {
        indicator: <InfoIcon />,
        variant: "danger",
      });
    },
  });

  const testMutation = $api.useMutation("post", "/api/notifications/test", {
    onSuccess() {
      toast(t("actionSuccess"), {
        indicator: <Check />,
        variant: "success",
      });
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
      apiUrl: configQuery.data?.config?.apiUrl || "",
      apiSecret: configQuery.data?.config?.apiSecret || "",
    },
    validationSchema: yup.object({
      apiUrl: yup.string().url().required(),
      apiSecret: yup.string().required(),
    }),
    onSubmit(values) {
      configMutation.mutate({
        baseUrl: authData.apiUrl,
        headers: {
          Authorization: `Bearer ${authData.token}`,
        },
        body: {
          apiUrl: values.apiUrl,
          apiSecret: values.apiSecret,
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
    <CardWithTitle icon="bell-ring" title={t("notifications")}>
      {!configQuery.data?.isSet ? (
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

      <form
        onSubmit={formik.handleSubmit}
        className="flex flex-col gap-[0.5rem] p-[1rem]"
      >
        <ValidatedTextField
          formik={formik}
          name="apiUrl"
          textFieldProps={{
            isRequired: true,
          }}
          labelProps={{ children: t("apiUrl") }}
        ></ValidatedTextField>
        <ValidatedTextField
          formik={formik}
          name="apiSecret"
          labelProps={{ children: t("apiSecret") }}
          inputProps={{
            type: isVisible ? "text" : "password",
          }}
          textFieldProps={{
            isRequired: true,
          }}
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
          isPending={configMutation.isPending}
          className="mt-[1rem]"
        >
          {t("save")}
          <Save></Save>
        </Button>

        {configQuery.data?.isSet ? (
          <>
            <Separator className="my-[1rem]"></Separator>

            <Button
              fullWidth
              variant="outline"
              isPending={testMutation.isPending}
              className="text-accent"
              onPress={() =>
                testMutation.mutate({
                  baseUrl: authData.apiUrl,
                  headers: {
                    Authorization: `Bearer ${authData.token}`,
                  },
                })
              }
            >
              {t("test")}
              <BellRing></BellRing>
            </Button>
          </>
        ) : null}
      </form>
    </CardWithTitle>
  );
}
