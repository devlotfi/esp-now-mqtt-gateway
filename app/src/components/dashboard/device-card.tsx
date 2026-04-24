import { useTranslation } from "react-i18next";
import { $api } from "../../api/openapi-client";
import { useContext } from "react";
import { AuthContext } from "../../context/auth-context";
import { Button, toast } from "@heroui/react";
import { InfoIcon, Power } from "lucide-react";
import ChipSVG from "../svg/ChipSVG";
import DataRow from "../data-row";
import { formatTime } from "../../utils/format-time";
import type { paths } from "../../__generated__/schema";
import { AppContext } from "../../context/app-context";
import CardWithTitle from "../card-with-header";

export default function DeviceCard({
  data,
}: {
  data: paths["/api/device/status"]["get"]["responses"]["200"]["content"]["application/json"]["general"];
}) {
  const { t } = useTranslation();
  const { rebootModalState } = useContext(AppContext);
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");

  const rebootMutation = $api.useMutation("post", "/api/device/reboot", {
    onSuccess() {
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

  return (
    <CardWithTitle icon="microchip" title={t("device")}>
      <div className="flex flex-col items-center py-[1rem]">
        <ChipSVG className="h-[5rem]"></ChipSVG>
      </div>

      <div className="flex flex-col p-[1rem]">
        <DataRow name={t("uptime")} value={formatTime(data.uptime)}></DataRow>
        <DataRow
          name={t("cpuTemperature")}
          value={<div className="flex">{data.cpuTemp} °C</div>}
        ></DataRow>
        <DataRow name={t("time")} value={data.time}></DataRow>

        <Button
          fullWidth
          variant="outline"
          isPending={rebootMutation.isPending}
          className="mt-[1rem] bg-background text-danger"
          onPress={() =>
            rebootMutation.mutate({
              baseUrl: authData.apiUrl,
              headers: {
                Authorization: `Bearer ${authData.token}`,
              },
            })
          }
        >
          {t("reboot")}
          <Power></Power>
        </Button>
      </div>
    </CardWithTitle>
  );
}
