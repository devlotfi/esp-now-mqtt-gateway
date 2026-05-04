import { Button, Card, Chip, useOverlayState } from "@heroui/react";
import { Eye, EyeOff, Trash } from "lucide-react";
import { useTranslation } from "react-i18next";
import type { paths } from "../../__generated__/schema";
import DataRow from "../data-row";
import { useState } from "react";
import DeleteSleepyPeerModal from "./delete-sleepy-peer-modal";

interface SleepyPeerComponentProps {
  sleepyPeer: paths["/api/sleepy-peers"]["get"]["responses"]["200"]["content"]["application/json"]["sleepyPeers"][number];
}

export default function SleepyPeerComponent({
  sleepyPeer,
}: SleepyPeerComponentProps) {
  const { t } = useTranslation();

  const deleteSleepyPeerModalState = useOverlayState();

  const [isVisible, setIsVisible] = useState(false);
  const toggleVisibility = () => setIsVisible(!isVisible);

  return (
    <>
      <DeleteSleepyPeerModal
        state={deleteSleepyPeerModalState}
        sleepyPeer={sleepyPeer}
      ></DeleteSleepyPeerModal>

      <Card>
        <Card.Header className="flex-row justify-between items-center">
          <Chip color="accent" className="text-[11pt]">
            {sleepyPeer.name}
          </Chip>

          <div className="flex items-center gap-[0.5rem]">
            <Button
              isIconOnly
              variant="outline"
              className="bg-background text-danger"
              onPress={() => deleteSleepyPeerModalState.open()}
            >
              <Trash></Trash>
            </Button>
          </div>
        </Card.Header>
        <Card.Content>
          <DataRow name="ID" value={sleepyPeer.id} fold></DataRow>
          <DataRow name="MAC" value={sleepyPeer.mac} fold></DataRow>
          <DataRow
            name="LMK"
            value={
              <div className="flex items-center gap-[0.5rem]">
                <Button
                  isIconOnly
                  size="sm"
                  variant="outline"
                  className="bg-background rounded-full"
                  onPress={toggleVisibility}
                >
                  {isVisible ? <EyeOff></EyeOff> : <Eye></Eye>}
                </Button>
                <div className="flex">
                  {isVisible
                    ? sleepyPeer.lmk
                    : "................................"}
                </div>
              </div>
            }
            fold
          ></DataRow>
          <DataRow
            name={t("commandTopic")}
            value={sleepyPeer.commandTopic}
            fold
          ></DataRow>
          <DataRow
            name={t("dataTopic")}
            value={sleepyPeer.dataTopic}
            fold
          ></DataRow>
        </Card.Content>
      </Card>
    </>
  );
}
