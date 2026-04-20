import {
  Alert,
  Button,
  Card,
  Label,
  Skeleton,
  Switch,
  toast,
} from "@heroui/react";
import { useContext, useState } from "react";
import { useTranslation } from "react-i18next";
import { useFormik } from "formik";
import { EyeOff, Eye, Save, InfoIcon } from "lucide-react";
import ValidatedTextField from "../validated-text-field";
import * as yup from "yup";
import { $api } from "../../api/openapi-client";
import { useQueryClient } from "@tanstack/react-query";
import { AuthContext } from "../../context/auth-context";
import CardWithTitle from "../card-with-header";
import { AppContext } from "../../context/app-context";

export default function MqttSettings() {
  const { t } = useTranslation();
  const { rebootModalState } = useContext(AppContext);
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");
  const queryClient = useQueryClient();

  const configQuery = $api.useQuery("get", "/api/mqtt", {
    baseUrl: authData.apiUrl,
    headers: {
      Authorization: `Bearer ${authData.token}`,
    },
  });

  const configMutation = $api.useMutation("post", "/api/mqtt", {
    onSuccess() {
      queryClient.resetQueries({
        queryKey: ["get", "/api/mqtt"],
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
      clientId: configQuery.data?.config?.clientId || "",
      url: configQuery.data?.config?.url || "",
      useAuth: configQuery.data?.config?.useAuth || true,
      username: configQuery.data?.config?.username || "",
      password: configQuery.data?.config?.password || "",
    },
    validationSchema: yup.object({
      clientId: yup.string().required(),
      url: yup
        .string()
        .required()
        .test("is-mqtt-url", "Invalid MQTT URL", (value) => {
          if (!value) return false;
          try {
            const url = new URL(value);
            return ["mqtt:", "mqtts:"].includes(url.protocol);
          } catch {
            return false;
          }
        }),
      useAuth: yup.bool(),
      username: yup.string().when("useAuth", {
        is: true,
        then: (schema) => schema.required(),
        otherwise: (schema) => schema.notRequired(),
      }),
      password: yup.string().when("useAuth", {
        is: true,
        then: (schema) => schema.required(),
        otherwise: (schema) => schema.notRequired(),
      }),
    }),
    onSubmit(values) {
      console.log(values);

      configMutation.mutate({
        baseUrl: authData.apiUrl,
        headers: {
          Authorization: `Bearer ${authData.token}`,
        },
        body: {
          clientId: values.clientId,
          url: values.url,
          useAuth: values.useAuth,
          username: values.useAuth ? values.username : undefined,
          password: values.useAuth ? values.password : undefined,
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
    <CardWithTitle icon="chevrons-left-right-ellipsis" title="MQTT">
      {!configQuery.data?.isSet ? (
        <div className="flex px-[0.5rem] pt-[0.5rem]">
          <Alert status="danger">
            <Alert.Indicator />
            <Alert.Content>
              <Alert.Title>{t("noMqttConfig1")}</Alert.Title>
              <Alert.Description>{t("noMqttConfig2")}</Alert.Description>
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
          name="clientId"
          textFieldProps={{
            isRequired: true,
          }}
          labelProps={{ children: t("clientId") }}
        ></ValidatedTextField>
        <ValidatedTextField
          formik={formik}
          name="url"
          textFieldProps={{
            isRequired: true,
          }}
          labelProps={{ children: "URL" }}
        ></ValidatedTextField>

        <Switch
          isSelected={formik.values.useAuth}
          onChange={(value) => formik.setFieldValue("useAuth", value)}
        >
          <Switch.Control>
            <Switch.Thumb />
          </Switch.Control>
          <Label className="text-sm">{t("useAuthenthication")}</Label>
        </Switch>
        {formik.values.useAuth ? (
          <>
            <ValidatedTextField
              formik={formik}
              name="username"
              labelProps={{ children: t("username") }}
              textFieldProps={{
                isRequired: true,
              }}
            ></ValidatedTextField>
            <ValidatedTextField
              formik={formik}
              name="password"
              labelProps={{ children: t("password") }}
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
          </>
        ) : null}

        <Button
          fullWidth
          type="submit"
          isPending={configMutation.isPending}
          className="mt-[1rem]"
        >
          {t("saveAndReboot")}
          <Save></Save>
        </Button>
      </form>
    </CardWithTitle>
  );
}
