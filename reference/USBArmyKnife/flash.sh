#!/usr/bin/env bash

esptool \
  --chip esp32s3 \
  --baud 921600 \
  --before default-reset \
  --after hard-reset write-flash -z \
  --flash-mode dio \
  --flash-freq 80m \
  --flash-size 16MB \
  0x0 firmware.bin
