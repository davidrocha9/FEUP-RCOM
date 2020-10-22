#include "alarm.h"

void alarm_handler(int signal) {
    /**
     * update variables 
     */
    app.numTries++;
    app.timeouts++;
    app.alarmFlag = 1;
}

void startAlarm() {
    struct sigaction action;
    action.sa_handler = &alarm_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGALRM, &action, NULL) < -1) {
        fprintf(stderr,"Unable to install SIGALRM handler\n");
        exit(1);
    }
    alarm(MAX_TIMEOUTS);
}

void stopAlarm() {
    struct sigaction act;
    act.sa_handler = NULL;
    sigaction(SIGALRM, &act, NULL);
    alarm(0);
}