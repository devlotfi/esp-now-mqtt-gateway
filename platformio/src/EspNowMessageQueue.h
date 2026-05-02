#pragma once

#include <esp_now.h>
#include "Properties.h"
#include "Vars.h"

struct QueueMessage
{
  uint8_t peer[6];
  uint8_t data[ESP_NOW_MAX_PAYLOAD_SIZE];
  size_t len;
  uint8_t retries;
};

void senderTask(void *parameter);

// Queue handle
QueueHandle_t sendQueue = nullptr;
StaticQueue_t *queueControl = nullptr;
uint8_t *queueStorage = nullptr;

bool initQueue()
{
  size_t itemSize = sizeof(QueueMessage);
  queueControl = (StaticQueue_t *)malloc(sizeof(StaticQueue_t));
  if (!queueControl)
    return false;

  // Queue items storage (PSRAM)
  queueStorage = (uint8_t *)malloc(ESP_NOW_MAX_QUEUE_SIZE * itemSize);
  if (!queueStorage)
  {
    free(queueControl);
    return false;
  }

  // Create queue using PSRAM-backed memory
  sendQueue = xQueueCreateStatic(
      ESP_NOW_MAX_QUEUE_SIZE,
      itemSize,
      queueStorage,
      queueControl);

  xTaskCreatePinnedToCoreWithCaps(
      senderTask,
      "sender_task",
      4096,
      nullptr,
      1,
      nullptr,
      0,
      MALLOC_CAP_SPIRAM);

  return sendQueue != nullptr;
}

bool enqueueMessage(const uint8_t *peer, const uint8_t *data, size_t len)
{
  if (!sendQueue || len > ESP_NOW_MAX_PAYLOAD_SIZE)
    return false;

  QueueMessage msg;

  memcpy(msg.peer, peer, 6);
  memcpy(msg.data, data, len);

  msg.len = len;
  msg.retries = 0;

  return xQueueSend(sendQueue, &msg, 0) == pdTRUE;
}

void senderTask(void *parameter)
{
  QueueMessage msg;

  while (true)
  {
    if (xQueueReceive(sendQueue, &msg, portMAX_DELAY) == pdTRUE)
    {
      bool delivered = false;

      while (msg.retries < ESP_NOW_MAX_RETRIES && !delivered)
      {
        esp_err_t err = esp_now_send(
            msg.peer,
            msg.data,
            msg.len);

        // Local stack rejected it
        if (err != ESP_OK)
        {
          msg.retries++;
          vTaskDelay(pdMS_TO_TICKS(50));
          continue;
        }

        delivered = true;
      }

      if (!delivered)
      {
        Serial.println("Message dropped after retries");
      }
    }
  }
}