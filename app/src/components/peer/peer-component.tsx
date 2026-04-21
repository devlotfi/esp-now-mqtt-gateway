import { Button, Card, Chip, useOverlayState } from "@heroui/react";
import { Eye, EyeOff, Mails, Trash } from "lucide-react";
import { useTranslation } from "react-i18next";
import type { paths } from "../../__generated__/schema";
import DataRow from "../data-row";
import DeletePeerModal from "./delete-peer-modal";
import TopicsModal from "./topics-modal";
import { useState } from "react";

interface PeerComponentProps {
  peer: paths["/api/esp-now/peers"]["get"]["responses"]["200"]["content"]["application/json"]["peers"][number];
}

export default function PeerComponent({ peer }: PeerComponentProps) {
  const { t } = useTranslation();

  const deletePeerModalState = useOverlayState();
  const topicsModalState = useOverlayState();

  const [isVisible, setIsVisible] = useState(false);
  const toggleVisibility = () => setIsVisible(!isVisible);

  return (
    <>
      <TopicsModal state={topicsModalState} peer={peer}></TopicsModal>
      <DeletePeerModal
        state={deletePeerModalState}
        peer={peer}
      ></DeletePeerModal>

      <Card>
        <Card.Header className="flex-row justify-between items-center">
          <Chip color="accent" className="text-[11pt]">
            {peer.name}
          </Chip>

          <div className="flex items-center gap-[0.5rem]">
            <Button
              variant="outline"
              className="bg-background"
              onPress={() => topicsModalState.open()}
            >
              {t("topics")}
              <Mails></Mails>
            </Button>
            <Button
              isIconOnly
              variant="outline"
              className="bg-background text-danger"
              onPress={() => deletePeerModalState.open()}
            >
              <Trash></Trash>
            </Button>
          </div>
        </Card.Header>
        <Card.Content>
          <DataRow name="ID" value={peer.id} fold></DataRow>
          <DataRow name="MAC" value={peer.mac} fold></DataRow>
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
                  {isVisible ? peer.lmk : "................................"}
                </div>
              </div>
            }
            fold
          ></DataRow>
        </Card.Content>
      </Card>
    </>
  );
}
