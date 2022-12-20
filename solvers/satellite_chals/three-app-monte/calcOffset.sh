#!/bin/bash
START=$(nm -gD spaceflag.so | awk '/appStart/ {print $1}')
START=$(printf '0x%x\n' "$((16#${START}))")
TARGET=$(nm -gD spaceflag.so | awk '/Send_Token/ {print $1}')
TARGET=$(printf '0x%x\n' "$((16#${TARGET}))")
echo "START  ${START}"
echo "TARGET ${TARGET}"
OFFSET=$(printf '0x%x\n' "$((TARGET-START))")
echo "OFFSET ${OFFSET}"