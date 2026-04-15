import { Button, Card, Skeleton } from "@heroui/react";
import { useState } from "react";
import { SectionTitle } from "../../components/section-title";
import { useTranslation } from "react-i18next";
import { useFormik } from "formik";
import { EyeOff, Eye, Save } from "lucide-react";
import ValidatedTextField from "../validated-text-field";
import * as yup from "yup";

export default function NotificationsSettings() {
  const { t } = useTranslation();

  const formik = useFormik({
    enableReinitialize: true,
    initialValues: {
      apiUrl: "",
      apiSecret: "",
    },
    validationSchema: yup.object({
      apiUrl: yup.string().url().required(),
      apiSecret: yup.string().required(),
    }),
    onSubmit() {},
  });

  const [isVisible, setIsVisible] = useState(false);
  const toggleVisibility = () => setIsVisible(!isVisible);

  if (!true)
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
    <Card>
      <Card.Content className="flex flex-col gap-[0.7rem]">
        <SectionTitle icon="bell-ring">{t("notifications")}</SectionTitle>

        <form
          onSubmit={formik.handleSubmit}
          className="flex flex-col gap-[0.5rem]"
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

          <Button fullWidth type="submit" className="mt-[1rem]">
            {t("save")}
            <Save></Save>
          </Button>
        </form>
      </Card.Content>
    </Card>
  );
}
