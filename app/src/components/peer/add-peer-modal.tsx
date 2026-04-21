import {
  Button,
  Form,
  Modal,
  toast,
  type UseOverlayStateReturn,
} from "@heroui/react";
import { useQueryClient } from "@tanstack/react-query";
import { Eye, EyeOff, InfoIcon, Plus, RefreshCw } from "lucide-react";
import { useTranslation } from "react-i18next";
import * as yup from "yup";
import { useFormik } from "formik";
import ValidatedTextField from "../validated-text-field";
import { $api } from "../../api/openapi-client";
import { useContext, useEffect, useState } from "react";
import { AuthContext } from "../../context/auth-context";
import { generateUnicastMac } from "../../utils/generate-unicast-mac";
import { generateEspNowKey } from "../../utils/generate-esp-now-key";

interface AddPeerModalProps {
  state: UseOverlayStateReturn;
}

export default function AddPeerModal({ state }: AddPeerModalProps) {
  const { t } = useTranslation();
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");
  const queryClient = useQueryClient();

  const { mutate, isPending } = $api.useMutation("post", "/api/esp-now/peers", {
    onError() {
      toast(t("error"), {
        indicator: <InfoIcon />,
        variant: "danger",
      });
    },
    onSuccess() {
      queryClient.resetQueries({
        queryKey: ["get", "/api/esp-now/peers"],
      });
      state.close();
    },
  });

  const formik = useFormik({
    initialValues: {
      name: "",
      mac: "",
      lmk: "",
    },
    validationSchema: yup.object({
      name: yup.string().required(),
      mac: yup
        .string()
        .matches(
          /^([0-9A-Fa-f][02468ACEace])([:. -]?)([0-9A-Fa-f]{2}\2){4}[0-9A-Fa-f]{2}$/,
          "Must be a valid unicast MAC address",
        )
        .required(),
      lmk: yup
        .string()
        .matches(/^[A-F0-9]{32}$/)
        .required(),
    }),
    onSubmit(values) {
      mutate({
        body: {
          name: values.name,
          mac: values.mac,
          lmk: values.lmk,
        },
        baseUrl: authData.apiUrl,
        headers: {
          Authorization: `Bearer ${authData.token}`,
        },
      });
    },
  });

  useEffect(() => {
    if (state.isOpen === false) {
      formik.resetForm();
    }
  }, [state.isOpen]);

  const [isVisible, setIsVisible] = useState(false);
  const toggleVisibility = () => setIsVisible(!isVisible);

  return (
    <Modal.Backdrop
      isOpen={state.isOpen}
      onOpenChange={state.setOpen}
      variant="blur"
    >
      <Modal.Container placement="center">
        <Modal.Dialog>
          <Modal.CloseTrigger />
          <Modal.Header>
            <Modal.Icon className="bg-accent-soft text-accent-soft-foreground">
              <Plus className="size-5" />
            </Modal.Icon>
            <Modal.Heading>{t("addPeer")}</Modal.Heading>
          </Modal.Header>
          <Modal.Body className="p-[0.3rem]">
            <Form
              onSubmit={formik.handleSubmit}
              className="flex flex-col gap-[0.5rem]"
            >
              <ValidatedTextField
                formik={formik}
                name="name"
                textFieldProps={{ isRequired: true }}
                labelProps={{ children: t("name") }}
              ></ValidatedTextField>
              <ValidatedTextField
                formik={formik}
                name="mac"
                textFieldProps={{ isRequired: true }}
                labelProps={{ children: "MAC" }}
                suffix={
                  <Button
                    isIconOnly
                    variant="ghost"
                    size="sm"
                    onPress={() => {
                      formik.setFieldValue("mac", generateUnicastMac());
                    }}
                  >
                    <RefreshCw></RefreshCw>
                  </Button>
                }
              ></ValidatedTextField>
              <ValidatedTextField
                formik={formik}
                name="lmk"
                textFieldProps={{ isRequired: true }}
                labelProps={{ children: "LMK" }}
                inputProps={{
                  type: isVisible ? "text" : "password",
                }}
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
                        formik.setFieldValue("lmk", generateEspNowKey());
                      }}
                    >
                      <RefreshCw></RefreshCw>
                    </Button>
                  </div>
                }
              ></ValidatedTextField>

              <Button
                fullWidth
                isPending={isPending}
                type="submit"
                className="mt-[1rem]"
              >
                {t("add")}
              </Button>
            </Form>
          </Modal.Body>
        </Modal.Dialog>
      </Modal.Container>
    </Modal.Backdrop>
  );
}
