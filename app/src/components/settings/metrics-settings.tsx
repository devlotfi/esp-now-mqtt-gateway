import { Alert, Button, Card, Skeleton, toast } from "@heroui/react";
import { useContext, useState } from "react";
import { useTranslation } from "react-i18next";
import { useFormik } from "formik";
import { EyeOff, Eye, Save, Check, InfoIcon } from "lucide-react";
import ValidatedTextField from "../validated-text-field";
import * as yup from "yup";
import { $api } from "../../api/openapi-client";
import { useQueryClient } from "@tanstack/react-query";
import { AuthContext } from "../../context/auth-context";
import CardWithTitle from "../card-with-header";
import GrafanaLogo from "../../assets/grafana-logo.svg";

export default function MetricsSettings() {
  const { t } = useTranslation();
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");
  const queryClient = useQueryClient();

  const configQuery = $api.useQuery("get", "/api/grafana", {
    baseUrl: authData.apiUrl,
    headers: {
      Authorization: `Bearer ${authData.token}`,
    },
  });

  const configMutation = $api.useMutation("post", "/api/grafana", {
    onSuccess() {
      queryClient.resetQueries({
        queryKey: ["get", "/api/grafana"],
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

  const formik = useFormik({
    enableReinitialize: true,
    initialValues: {
      url: configQuery.data?.config?.url || "",
      instanceId: configQuery.data?.config?.instanceId || "",
      apiKey: configQuery.data?.config?.apiKey || "",
    },
    validationSchema: yup.object({
      url: yup.string().url().required(),
      instanceId: yup.string().required(),
      apiKey: yup.string().required(),
    }),
    onSubmit(values) {
      configMutation.mutate({
        baseUrl: authData.apiUrl,
        headers: {
          Authorization: `Bearer ${authData.token}`,
        },
        body: {
          url: values.url,
          instanceId: values.instanceId,
          apiKey: values.apiKey,
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
    <CardWithTitle icon="chart-line" title={t("metrics")}>
      <div className="flex px-[0.5rem] pt-[0.5rem]">
        <Alert className="p-[0.3rem] bg-surface">
          <Alert.Indicator>
            <img src={GrafanaLogo} alt="grafana" className="h-[2rem]" />
          </Alert.Indicator>
          <Alert.Content className="justify-center">
            <Alert.Title className="text-[12pt]">Grafana</Alert.Title>
          </Alert.Content>
        </Alert>
      </div>

      {!configQuery.data?.isSet ? (
        <div className="flex px-[0.5rem] pt-[0.5rem]">
          <Alert status="danger">
            <Alert.Indicator />
            <Alert.Content>
              <Alert.Title>{t("noMetricsConfig1")}</Alert.Title>
              <Alert.Description>{t("noMetricsConfig2")}</Alert.Description>
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
          name="url"
          textFieldProps={{
            isRequired: true,
          }}
          labelProps={{ children: "URL" }}
        ></ValidatedTextField>
        <ValidatedTextField
          formik={formik}
          name="instanceId"
          textFieldProps={{
            isRequired: true,
          }}
          labelProps={{ children: t("instanceId") }}
        ></ValidatedTextField>
        <ValidatedTextField
          formik={formik}
          name="apiKey"
          labelProps={{ children: t("apiKey") }}
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
      </form>
    </CardWithTitle>
  );
}
