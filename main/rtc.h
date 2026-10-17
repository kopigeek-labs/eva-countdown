// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "clock_time.h"
bool rtc_init();
bool rtc_read(eva::DateTime &time);
bool rtc_set(const eva::DateTime &time);
