import { Button, Card, toast } from "@heroui/react";
import { useContext, useState } from "react";
import { useTranslation } from "react-i18next";
import { Check, Eye, EyeOff, InfoIcon, Save } from "lucide-react";
import { SectionTitle } from "../../components/section-title";
import { useFormik } from "formik";
import * as yup from "yup";
import { $api } from "../../api/openapi-client";
import ValidatedTextField from "../../components/validated-text-field";
import { AuthContext } from "../../context/auth-context";

export default function PasswordSettings() {
  const { t } = useTranslation();
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");

  const { mutate, isPending } = $api.useMutation(
    "post",
    "/api/auth/set-password",
    {
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
    },
  );

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
