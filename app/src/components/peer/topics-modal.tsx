import {
  Button,
  Modal,
  Surface,
  toast,
  type UseOverlayStateReturn,
} from "@heroui/react";
import type { paths } from "../../__generated__/schema";
import { Check, InfoIcon, Mails, Plus, Save, Trash } from "lucide-react";
import { useTranslation } from "react-i18next";
import { $api } from "../../api/openapi-client";
import { useContext, useEffect, useState } from "react";
import { AuthContext } from "../../context/auth-context";
import LoadingScreen from "../loading-screen";
import ErrorScreen from "../error-screen";
import EmptySVG from "../svg/EmptySVG";
import { useFormik } from "formik";
import * as yup from "yup";
import ValidatedTextField from "../validated-text-field";
import { useQueryClient } from "@tanstack/react-query";
import { sameElements } from "../../utils/same-elements";

interface TopicsModalProps {
  state: UseOverlayStateReturn;
  peer: paths["/peers"]["get"]["responses"]["200"]["content"]["application/json"]["peers"][number];
}

function TopicComponent({
  topic,
  deleteTopic,
}: {
  topic: string;
  deleteTopic: (topic: string) => void;
}) {
  return (
    <Surface className="flex items-center justify-between border border-border rounded-xl p-[0.5rem]">
      <div className="flex">{topic}</div>
      <Button
        isIconOnly
        size="sm"
        variant="outline"
        className="text-danger rounded-full"
        onPress={() => deleteTopic(topic)}
      >
        <Trash></Trash>
      </Button>
    </Surface>
  );
}

export default function TopicsModal({ state, peer }: TopicsModalProps) {
  const { t } = useTranslation();
  const { authData } = useContext(AuthContext);
  if (!authData) throw new Error("Missing auth data");
  const queryClient = useQueryClient();

  const topicsQuery = $api.useQuery(
    "get",
    "/topics/{id}",
    {
      params: {
        path: {
          id: peer.id,
        },
      },
      baseUrl: authData.apiUrl,
      headers: {
        Authorization: `Bearer ${authData.token}`,
      },
    },
    {
      enabled: state.isOpen,
      refetchOnWindowFocus: false,
      refetchOnReconnect: false,
    },
  );
  const [topics, setTopics] = useState<string[]>([]);
  useEffect(() => {
    if (topicsQuery.data?.topicList) {
      setTopics(topicsQuery.data.topicList);
    }
  }, [topicsQuery.data]);

  const setTopicsMutation = $api.useMutation("post", "/topics/{id}", {
    onSuccess() {
      queryClient.resetQueries({
        queryKey: ["get", "/topics/{id}"],
      });
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
      topic: "",
    },
    validationSchema: yup.object({
      topic: yup.string().max(48).required(),
    }),
    onSubmit(values) {
      if (!topics.includes(values.topic)) {
        setTopics([...topics, values.topic]);
      }
    },
  });

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
              <Mails className="size-5" />
            </Modal.Icon>
            <Modal.Heading>{t("topicSubscriptions")}</Modal.Heading>
          </Modal.Header>
          <Modal.Body className="p-[0.3rem]">
            {topicsQuery.isLoading ? (
              <LoadingScreen></LoadingScreen>
            ) : topicsQuery.isError ? (
              <ErrorScreen></ErrorScreen>
            ) : (
              <div className="flex flex-col">
                <form onSubmit={formik.handleSubmit}>
                  <ValidatedTextField
                    formik={formik}
                    name="topic"
                    inputProps={{
                      className: "h-[2.7rem] pr-0",
                      placeholder: t("topic"),
                    }}
                    suffix={
                      <Button
                        isIconOnly
                        variant="primary"
                        size="sm"
                        type="submit"
                        className="rounded-full"
                      >
                        <Plus></Plus>
                      </Button>
                    }
                  ></ValidatedTextField>
                </form>

                {topics.length ? (
                  <div className="flex flex-col pt-[1rem]">
                    <div className="flex flex-col gap-[0.5rem]">
                      {topics.map((topic, index) => (
                        <TopicComponent
                          key={`${index}-${topic}`}
                          topic={topic}
                          deleteTopic={(topic) => {
                            setTopics(topics.filter((it) => it !== topic));
                          }}
                        ></TopicComponent>
                      ))}
                    </div>

                    {!sameElements(
                      topics,
                      topicsQuery.data?.topicList || [],
                    ) ? (
                      <Button
                        fullWidth
                        isPending={setTopicsMutation.isPending}
                        className="mt-[1rem]"
                        onPress={() =>
                          setTopicsMutation.mutate({
                            params: {
                              path: {
                                id: peer.id,
                              },
                            },
                            body: {
                              topicList: topics,
                            },
                            baseUrl: authData.apiUrl,
                            headers: {
                              Authorization: `Bearer ${authData.token}`,
                            },
                          })
                        }
                      >
                        {t("save")}
                        <Save></Save>
                      </Button>
                    ) : null}
                  </div>
                ) : (
                  <div className="flex flex-1 py-[2rem] text-center justify-center items-center flex-col gap-[1rem] px-[0.5rem]">
                    <EmptySVG className="h-[10rem]" />
                    <div className="flex text-[15pt] text-foreground font-bold">
                      {t("emptyList")}...
                    </div>
                  </div>
                )}
              </div>
            )}
          </Modal.Body>
        </Modal.Dialog>
      </Modal.Container>
    </Modal.Backdrop>
  );
}
