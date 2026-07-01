import {
  Autocomplete,
  Button,
  Card,
  EmptyState,
  Label,
  ListBox,
  SearchField,
  Skeleton,
  toast,
  useFilter,
} from "@heroui/react";
import { useContext } from "react";
import { useTranslation } from "react-i18next";
import { useFormik } from "formik";
import { Save, InfoIcon } from "lucide-react";
import * as yup from "yup";
import { $api } from "../../api/openapi-client";
import { useQueryClient } from "@tanstack/react-query";
import { AuthContext } from "../../context/auth-context";
import CardWithTitle from "../card-with-header";
import { AppContext } from "../../context/app-context";
import { tzTable } from "../../utils/tz-table";
import DataRow from "../data-row";

export default function TimezoneSettings() {
  const { t } = useTranslation();
  const { rebootModalState } = useContext(AppContext);
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");
  const queryClient = useQueryClient();
  const { contains } = useFilter({ sensitivity: "base" });

  const configQuery = $api.useQuery("get", "/api/timezone", {
    baseUrl: authData.apiUrl,
    headers: {
      Authorization: `Bearer ${authData.token}`,
    },
  });

  const configMutation = $api.useMutation("post", "/api/timezone", {
    onSuccess() {
      queryClient.resetQueries({
        queryKey: ["get", "/api/timezone"],
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
      iana: configQuery.data?.config.iana || "",
      timezonePosix: configQuery.data?.config.timezonePosix || "",
    },
    validationSchema: yup.object({
      iana: yup.string().required(),
      timezonePosix: yup.string().required(),
    }),
    onSubmit(values) {
      configMutation.mutate({
        baseUrl: authData.apiUrl,
        headers: {
          Authorization: `Bearer ${authData.token}`,
        },
        body: {
          iana: values.iana,
          timezonePosix: values.timezonePosix,
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
    <CardWithTitle icon="clock" title={t("timezone")}>
      <form
        onSubmit={formik.handleSubmit}
        className="flex flex-col gap-[0.5rem] p-[1rem]"
      >
        <Autocomplete
          placeholder={t("timezone")}
          selectionMode="single"
          value={formik.values.iana}
          onChange={(key) => {
            if (key) {
              formik.setFieldValue("iana", key.toString());
              formik.setFieldValue("timezonePosix", tzTable[key.toString()]);
            }
          }}
        >
          <Label>IANA</Label>
          <Autocomplete.Trigger>
            <Autocomplete.Value />
            <Autocomplete.Indicator />
          </Autocomplete.Trigger>
          <Autocomplete.Popover>
            <Autocomplete.Filter filter={contains}>
              <SearchField autoFocus name="search" variant="secondary">
                <SearchField.Group>
                  <SearchField.SearchIcon />
                  <SearchField.Input />
                  <SearchField.ClearButton />
                </SearchField.Group>
              </SearchField>
              <ListBox
                renderEmptyState={() => (
                  <EmptyState>{t("emptyList")}</EmptyState>
                )}
              >
                {Object.keys(tzTable).map((tzMapping) => (
                  <ListBox.Item
                    key={tzMapping}
                    id={tzMapping}
                    textValue={tzMapping}
                  >
                    {tzMapping}
                    <ListBox.ItemIndicator />
                  </ListBox.Item>
                ))}
              </ListBox>
            </Autocomplete.Filter>
          </Autocomplete.Popover>
        </Autocomplete>

        {formik.values.timezonePosix ? (
          <DataRow
            name={"POSIX TZ"}
            value={formik.values.timezonePosix}
          ></DataRow>
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
