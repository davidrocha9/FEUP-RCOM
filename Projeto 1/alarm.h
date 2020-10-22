#pragma once
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "ll.h"

#define MAX_TIMEOUTS 3

void alarm_handler(int signal);
void startAlarm();
void stopAlarm();