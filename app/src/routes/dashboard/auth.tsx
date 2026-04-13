import { createFileRoute } from "@tanstack/react-router";
import SectionHeader from "../../components/section-header";
import { useTranslation } from "react-i18next";
import { Button, Card, InfoIcon, toast } from "@heroui/react";
import { SectionTitle } from "../../components/section-title";
import { useFormik } from "formik";
import * as yup from "yup";
import { $api } from "../../api/openapi-client";
import { Check, Eye, EyeOff, RefreshCw, Save } from "lucide-react";
import { useContext, useState } from "react";
import { AuthContext } from "../../context/auth-context";
import ValidatedTextField from "../../components/validated-text-field";
import { generateEspNowKey } from "../../utils/generate-esp-now-key";

export const Route = createFileRoute("/dashboard/auth")({
  component: RouteComponent,
});

function PasswordOption() {
  const { t } = useTranslation();
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");

  const { mutate, isPending } = $api.useMutation("post", "/set-password", {
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
    initialValues: {
      password: "",
    },
    validationSchema: yup.object({
      password: yup.string().required(),
    }),
    onSubmit(values) {
      mutate({
        baseUrl: authData.apiUrl,
        headers: {
          Authorization: `Bearer ${authData.token}`,
        },
        body: {
          password: values.password,
        },
      });
    },
  });

  const [isVisible, setIsVisible] = useState(false);
  const toggleVisibility = () => setIsVisible(!isVisible);

  return (
    <Card>
      <Card.Content className="flex flex-col gap-[0.7rem]">
        <SectionTitle icon="key-round">{t("setPassword")}</SectionTitle>

        <form
          onSubmit={formik.handleSubmit}
          className="flex flex-col gap-[1rem]"
        >
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
            <Save></Save>
            {t("save")}
          </Button>
        </form>
      </Card.Content>
    </Card>
  );
}

function PMKOption() {
  const { t } = useTranslation();
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");

  const { mutate, isPending } = $api.useMutation("post", "/set-pmk", {
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
    initialValues: {
      pmk: "",
    },
    validationSchema: yup.object({
      pmk: yup
        .string()
        .matches(/^[A-F0-9]{32}$/)
        .required(),
    }),
    onSubmit(values) {
      mutate({
        baseUrl: authData.apiUrl,
        headers: {
          Authorization: `Bearer ${authData.token}`,
        },
        body: {
          pmk: values.pmk,
        },
      });
    },
  });

  const [isVisible, setIsVisible] = useState(false);
  const toggleVisibility = () => setIsVisible(!isVisible);

  return (
    <Card className="mt-[2rem]">
      <Card.Content className="flex flex-col gap-[0.7rem]">
        <SectionTitle icon="key-round">{t("setPMK")}</SectionTitle>

        <form
          onSubmit={formik.handleSubmit}
          className="flex flex-col gap-[1rem]"
        >
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
            isPending={isPending}
            className="mt-[1rem]"
          >
            <Save></Save>
            {t("save")}
          </Button>
        </form>
      </Card.Content>
    </Card>
  );
}

function RouteComponent() {
  const { t } = useTranslation();

  return (
    <div className="flex flex-1 flex-col items-center p-[1rem]">
      <div className="flex flex-col w-full max-w-screen-sm pb-[5rem]">
        <SectionHeader icon="user-key">{t("auth")}</SectionHeader>

        <PasswordOption></PasswordOption>
        <PMKOption></PMKOption>
      </div>
    </div>
  );
}
