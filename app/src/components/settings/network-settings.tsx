import {
  Button,
  Card,
  Label,
  ListBox,
  Select,
  Skeleton,
  toast,
} from "@heroui/react";
import { useContext } from "react";
import { useTranslation } from "react-i18next";
import { useFormik } from "formik";
import { Save, InfoIcon } from "lucide-react";
import ValidatedTextField from "../validated-text-field";
import * as yup from "yup";
import { $api } from "../../api/openapi-client";
import { useQueryClient } from "@tanstack/react-query";
import { AuthContext } from "../../context/auth-context";
import CardWithTitle from "../card-with-header";
import { AppContext } from "../../context/app-context";
import { isValidIPv4 } from "../../validation/is-valid-ipv4";
import { isValidPrivateIPv4 } from "../../validation/is-valid-private-ipv4";
import { isValidSubnetMask } from "../../validation/is-valid-subnet-mask";

export default function NetworkSettings() {
  const { t } = useTranslation();
  const { rebootModalState } = useContext(AppContext);
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");
  const queryClient = useQueryClient();

  const configQuery = $api.useQuery("get", "/api/network", {
    baseUrl: authData.apiUrl,
    headers: {
      Authorization: `Bearer ${authData.token}`,
    },
  });

  const configMutation = $api.useMutation("post", "/api/network", {
    onSuccess() {
      queryClient.resetQueries({
        queryKey: ["get", "/api/network"],
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
      ipAssignment: configQuery.data?.ipAssignment || "DHCP",
      ip:
        configQuery.data?.ipAssignment === "STATIC" &&
        configQuery.data?.staticConfig?.ip
          ? configQuery.data?.staticConfig?.ip
          : "",
      gateway:
        configQuery.data?.ipAssignment === "STATIC" &&
        configQuery.data?.staticConfig?.gateway
          ? configQuery.data?.staticConfig?.gateway
          : "",
      subnet:
        configQuery.data?.ipAssignment === "STATIC" &&
        configQuery.data?.staticConfig?.subnet
          ? configQuery.data?.staticConfig?.subnet
          : "",
      dns:
        configQuery.data?.ipAssignment === "STATIC" &&
        configQuery.data?.staticConfig?.dns
          ? configQuery.data?.staticConfig?.dns
          : "",
    },
    validationSchema: yup.object({
      ipAssignment: yup.string().oneOf(["DHCP", "STATIC"]).required(),
      ip: yup.string().when("ipAssignment", {
        is: "STATIC",
        then: (schema) =>
          schema
            .required()
            .test("ip", "Invalid IP", (value) =>
              isValidPrivateIPv4(value || ""),
            ),
        otherwise: (schema) => schema.notRequired(),
      }),
      gateway: yup.string().when("ipAssignment", {
        is: "STATIC",
        then: (schema) =>
          schema
            .required()
            .test("ip", "Invalid IP", (value) =>
              isValidPrivateIPv4(value || ""),
            ),
        otherwise: (schema) => schema.notRequired(),
      }),
      subnet: yup.string().when("ipAssignment", {
        is: "STATIC",
        then: (schema) =>
          schema
            .required()
            .test("ip", "Invalid Subnet", (value) =>
              isValidSubnetMask(value || ""),
            ),
        otherwise: (schema) => schema.notRequired(),
      }),
      dns: yup.string().when("ipAssignment", {
        is: "STATIC",
        then: (schema) =>
          schema
            .required()
            .test("ip", "Invalid IP", (value) => isValidIPv4(value || "")),
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
          ipAssignment: values.ipAssignment,
          staticConfig:
            values.ipAssignment === "STATIC"
              ? {
                  ip: values.ip,
                  gateway: values.gateway,
                  subnet: values.subnet,
                  dns: values.dns,
                }
              : undefined,
        },
      });
    },
  });

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
    <CardWithTitle icon="network" title={t("network")}>
      <form
        onSubmit={formik.handleSubmit}
        className="flex flex-col gap-[0.5rem] p-[1rem]"
      >
        <Select
          value={formik.values.ipAssignment}
          onChange={(value) => formik.setFieldValue("ipAssignment", value)}
        >
          <Label>{t("ipAssignment")}</Label>
          <Select.Trigger>
            <Select.Value />
            <Select.Indicator />
          </Select.Trigger>
          <Select.Popover>
            <ListBox>
              <ListBox.Item id={"DHCP"} textValue={"DHCP"}>
                DHCP
                <ListBox.ItemIndicator />
              </ListBox.Item>
              <ListBox.Item id={"STATIC"} textValue={"STATIC"}>
                STATIC
                <ListBox.ItemIndicator />
              </ListBox.Item>
            </ListBox>
          </Select.Popover>
        </Select>

        {formik.values.ipAssignment === "STATIC" ? (
          <>
            <ValidatedTextField
              formik={formik}
              name="ip"
              textFieldProps={{
                isRequired: true,
              }}
              labelProps={{ children: "IP" }}
            ></ValidatedTextField>
            <ValidatedTextField
              formik={formik}
              name="gateway"
              textFieldProps={{
                isRequired: true,
              }}
              labelProps={{ children: "Gateway" }}
            ></ValidatedTextField>
            <ValidatedTextField
              formik={formik}
              name="subnet"
              textFieldProps={{
                isRequired: true,
              }}
              labelProps={{ children: "Subnet" }}
            ></ValidatedTextField>
            <ValidatedTextField
              formik={formik}
              name="dns"
              textFieldProps={{
                isRequired: true,
              }}
              labelProps={{ children: "DNS" }}
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
