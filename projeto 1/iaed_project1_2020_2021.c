#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define TASK_STR            "task"
#define ACTIVITY1           "TO DO"
#define ACTIVITY1_ID        1
#define ACTIVITY2           "IN PROGRESS"
#define ACTIVITY2_ID        2
#define ACTIVITY3           "DONE"
#define ACTIVITY3_ID        3

#define INVALID_RETURN      -1
#define VALID_RETURN        0

#define ERROR_MANY_TASKS    "too many tasks"
#define ERROR_DUP_DESC      "duplicate description"
#define ERROR_INV_DUR       "invalid duration"

#define ERROR_INV_TIME      "invalid time"
#define ERROR_NO_TASK       "no such task"
#define ERROR_ALR_STARTED   "task already started"

#define ERROR_MANY_USERS    "too many users"
#define ERROR_USER_EXISTS   "user already exists"
#define ERROR_NO_USER       "no such user"

#define ERROR_MANY_ACTIV    "too many activities"
#define ERROR_DUP_ACTIV     "duplicate activity"
#define ERROR_NO_ACTIV      "no such activity"
#define ERROR_INV_DESC      "invalid description"

#define MAX_TASKS       10000
#define MAX_DESC_TASK   50

#define MAX_ACTIVS      10 /* 0-2 : TO DO, IN PROGRESS e DONE */
#define MAX_DESC_ACTIV  20 

#define MAX_DESC_USER   20 
#define MAX_USERS       50

/*---------------------------------------------------------------------------------------------------*/

#define MAX_LINE        1000



typedef struct task {
    int taskId; /* BETWEEN 1 && 10000*/
    char taskDescription[MAX_DESC_TASK + 1]; /* FROM 1 TO 50 CHARS, CANT BE ONLY '\0' */
    char taskUser[MAX_DESC_USER + 1];
    int taskActivityId; /* INDICE OF ACTIVITY IN saveActivityInfo ARRAY */
    int duration;
    int instant; /* EXACT TIME WHEN TASK LEFT THE 'TO DO' ACTIVITY */
} Task;

Task saveTaskInfo[MAX_TASKS + 1]; /* WE WONT USE INDICE 0, FROM 1 TO 10000 ; FOR TASK 1 => saveTaskInfo[1]; */
int nextTaskId = 1;




typedef struct activity {
    char activityDescription[MAX_DESC_ACTIV + 1]; /* FROM 1 TO 20 CHARS, CANT HAVE NON CAPITAL LETTERS */ 
} Activity;

Activity saveActivityInfo[MAX_ACTIVS + 1] = { /* WE WONT USE INDICE 0, FROM 1 TO 10 ; FOR ACTVITY 1 => saveActivityInfo[1]; */
    { "" }, /* not use it */
    { ACTIVITY1 },
    { ACTIVITY2 },
    { ACTIVITY3 },
};

int nextActivityId = 4;




typedef struct user {
    char userDescription[MAX_DESC_USER + 1]; /* FROM 1 TO 20 CHARS, CANT HAVE BLANK SPACES */
} User;

User saveUserInfo[MAX_USERS + 1]; /* WE WONT USE INDICE 0, FROM 1 TO 50 ; FOR USER 1 => saveUserInfo[1]; */
int nextUserId = 1; 


/* auxiliary arrays used to sort "things", and to hold read task ids*/
int refArray[MAX_TASKS], left, right; /* 0-9999 */
int intArray[MAX_TASKS];
int array[MAX_TASKS]; /* 0-9999*/


int time = 0;
char args[MAX_LINE];

void substring(char beginning[], int first, int lenght, char destination[]);


int new_task(char args[]);
int list_tasks(char args[]);
int advance_time(char args[]);
int add_list_user(char args[]);
int move_task(char args[]);
int add_list_activity(char args[]);
int lists_activity_tasks(char args[]);

/* main function */
int main()
{
    char cmd[MAX_LINE];
    /*int EXIT_SUCCESS = 0;*/

    while (fgets(cmd, sizeof(cmd), stdin))
    {
        switch (cmd[0])
        {
            case 'q':
                return 0;
            case 't':		/* adds a task to the task struct */
				new_task(&cmd[1]);
				break;
            case 'l':		 /*list tasks either all tasks either some tasks*/
				list_tasks(&cmd[1]);
				break;
            case 'n':        /*advances time*/
	            advance_time(&cmd[1]);
				break;
            case 'u':       /*lists users or adds a new one*/
                add_list_user(&cmd[1]);
                break;
            case 'm':        /*moves a task to another activity*/
                move_task(&cmd[1]);
                break;
            case 'd':       /*lists tasks in a certain activity*/
                lists_activity_tasks(&cmd[1]);
                break;
            case 'a':        /*lists activities or adds a new one*/
                add_list_activity(&cmd[1]);
                break;
            default:
				printf("'%c' is an invalid command character\n", cmd[0]);
		}
	}
	return 0;
}

/* returns the trimmed string of the given one */
void substring(char beginning[], int first, int length, char destination[])
{
    int i;
    /* deletes the ' ' from the beginning of the string */
    while (first < MAX_DESC_TASK) {
        if (beginning[first] == ' ' || beginning[first] == '\t' || beginning[first] == '\n')
            first++;
        else
            break;
    }
    /* copies until the lenght ends, the strings ends or a newline is found */
	for (i = 0; i < length - 1 && beginning[first] != '\0' && beginning[first] != '\n'; first++, i++)
		 destination[i] = beginning[first];

	/* adding the \0 */
	destination[i] = '\0';
}

void substring_no_spaces(char beginning[], int first, int length, char destination[])
{
    int i;

    /* deletes the ' ' from the beginning of the string */
    while (first < MAX_DESC_TASK) {
        if (beginning[first] == ' ' || beginning[first] == '\t' || beginning[first] == '\n')
            first++;
        else
            break;
    }
    /* copies until the lenght ends, the strings ends or a newline is found */
	for (i = 0; i < length - 1 && beginning[first] != '\0' && beginning[first] != '\n' && beginning[first] != ' '; first++, i++)
		 destination[i] = beginning[first];
		 
	/* adding the \0 */
	destination[i] = '\0';
}

void list_alphabetically(int left, int right)
{
    int i, j, v;
    /*inicialize the reference array with the indexs of the tasks*/
    for (i = left+1; i < right; i++) {
        v = refArray[i];
        j = i - 1;
        /*sort the refAarray using the insertion sort algorithm*/
        while (j >= left && strcmp(saveTaskInfo[v].taskDescription, saveTaskInfo[refArray[j]].taskDescription) < 0) {
            /*move elements of refArray[0..i-1] that are greater than v
            to one position ahead of their current position*/
            refArray[j+1] = refArray[j];
            j--;
        }
        /*insert the v at the hole that we opened above*/
        refArray[j+1] = v;
    }
}

void list_instant(int left, int right)
{
    int i, j, v;
    /*inicialize the intarray with the indexes of the tasks*/
    for (i = left+1; i < right; i++) {
        v = intArray[i];
        j = i - 1;
        /*sort the intAarray using the insertion sort algorithm*/
        while (j >= left && saveTaskInfo[v].instant < saveTaskInfo[intArray[j]].instant) {
            /*move elements of intArray[0..i-1] that are greater than v
            to one position ahead of their current position*/
            intArray[j+1] = intArray[j];
            j--;
        }
        /*insert the v at the hole that we opened above*/
        intArray[j+1] = v;
    }
}

/* creates a new task, t */
int new_task(char args[])
{
    int taskDuration, scanned, tid;
	char description[MAX_DESC_TASK + 1];

    /* check if we can create a new task */
    if (nextTaskId > MAX_TASKS) {
        printf("%s\n", ERROR_MANY_TASKS);
        return -1;
    }
	
    /* check if we already had the same description before */
    for (tid = 1; tid < nextTaskId; tid++ ) {
        if ((strcmp(saveTaskInfo[tid].taskDescription, description) == 0)) {
            printf("%s\n", ERROR_DUP_DESC);
            return -1;
        }
    }

	/* extract task duration */
	if ((sscanf(args, "%d%n", &taskDuration, &scanned) == 0) || taskDuration < 0) {
		printf("%s\n", ERROR_INV_DUR);
		return -1;
	}

	/* extract task description that is at the end of command line */
	substring(args, scanned, MAX_DESC_TASK + 1, description);

    /*get identifier for the next task */
    tid = nextTaskId++;
    saveTaskInfo[tid].taskId = tid;
    strcpy(saveTaskInfo[tid].taskDescription, description);
    saveTaskInfo[tid].taskActivityId = ACTIVITY1_ID;
    saveTaskInfo[tid].duration = taskDuration;

	/* no user and instant yet */
    printf("%s %d\n", TASK_STR, tid);
	return 0;
}

/* list certain tasks or all tasks */
int list_tasks(char args[])
{
	int taskId, scanned, count = 0, i;
	
	while (sscanf(args, "%d%n", &taskId, &scanned) == 1) {
		args = args + scanned;		/* advance the pointer the number of chars scanned by sscanf() */
		
		/* we ignore the ids that exceed MAX_TASK_IDS */
		if (count < MAX_TASKS) {
			array[count] = taskId;
			count++;
		}
	}

	if (count == 0) {
        for (i = 0; i < nextTaskId-1; i++ ) {
            refArray[i] = saveTaskInfo[i+1].taskId;
        }
        list_alphabetically(0, nextTaskId - 1);
        for (i= 0; i < nextTaskId-1; i++) {
            printf("%d %s #%d %s\n", saveTaskInfo[refArray[i]].taskId, saveActivityInfo[saveTaskInfo[refArray[i]].taskActivityId].activityDescription,
                                    saveTaskInfo[refArray[i]].duration, saveTaskInfo[refArray[i]].taskDescription);
        }
	}

    else { 
        /* list the tasks with the ids in the "taskIds" array */
		for (i = 0; i < count; i++) {
            if ((array[i] >= nextTaskId) || (array[i] <= 0)) {
                printf("%d: %s\n", array[i], ERROR_NO_TASK);
                return -1;
            }
        }
        for (i = 0; i < count; i++) {
            printf("%d %s #%d %s\n", saveTaskInfo[array[i]].taskId, saveActivityInfo[saveTaskInfo[array[i]].taskActivityId].activityDescription,
                                    saveTaskInfo[array[i]].duration, saveTaskInfo[array[i]].taskDescription);
	    }
    }
	return 0;
}

/* advances or shows current time */
int advance_time(char args[])
{
	int duration;
	
	/* extract duration of time to advance */
	if (sscanf(args, "%d", &duration) != 1 || duration < 0) {
		printf("%s\n", ERROR_INV_TIME);
		return -1;
	}
    time += duration;
    printf("%d\n", time);
	return 0;
}

/* adds a user or lists them */
int add_list_user(char args[])
{
    char descUser[MAX_DESC_USER + 1];
    int i, vazio;

    /* check if there is a username and truncate it to 20 chars*/
    substring_no_spaces(args, 0, MAX_DESC_USER + 1, descUser);
    vazio = strcmp(descUser, "");

    if (vazio != 0) {
        /* see if we can create a new user (max 50) */      
        for (i=0; i < nextUserId; i++) {
            if ((strcmp(saveUserInfo[i].userDescription, descUser) == 0)) {
                printf("%s\n", ERROR_USER_EXISTS);
                return -1;
            }
        }

        if (nextUserId > MAX_USERS) {
            printf("%s\n", ERROR_MANY_USERS);
            return -1;
        }
        /* no error found, we can create a new user */
        strcpy(saveUserInfo[nextUserId].userDescription, descUser);
        nextUserId++;
        return 0;
    }

    for (i = 1; i < nextUserId; i++) {
        /* by creation order, 1-50 */
        printf("%s\n", saveUserInfo[i].userDescription);
    }
    return 0;
}

/* moves a task from an activity to another */
int move_task(char args[])
{
    int taskId, scanned, i;
    int duration, slack;
    int activityId;
    char userDesc[MAX_DESC_USER + 1], descActiv[MAX_DESC_ACTIV + 1];

    if (sscanf(args, "%d%20s%n", &taskId, userDesc, &scanned) != 2) {
		printf("%s\n", ERROR_NO_TASK);
		return -1;
	}

    /* getting the activity string, descActiv*/
    substring(args, scanned, MAX_DESC_ACTIV + 1, descActiv);

    /* validating the task id */
    if (taskId >= nextTaskId || taskId <= 0) {
        printf("%s\n", ERROR_NO_TASK);
		return -1;
    }

    /* validating the activity description */
    for (activityId = 1; activityId < nextActivityId; activityId++) {
        if (strcmp(saveActivityInfo[activityId].activityDescription, descActiv) == 0) {
            break;
        }
    }

    /* seeing if we want to restart an already started task */
    if (saveTaskInfo[taskId].taskActivityId != ACTIVITY1_ID && activityId == ACTIVITY1_ID) {
        printf("%s\n", ERROR_ALR_STARTED);
        return -1;
    }

    /* validating the user name */
    for (i = 1; i < nextUserId; i++) {
        if (strcmp(saveUserInfo[i].userDescription, userDesc) == 0) {
            break;
        }
    }
    if (i == nextUserId) {
        printf("%s\n", ERROR_NO_USER);
        return -1;
    }

    /*returning the error */
    if (activityId == nextActivityId) {
        printf("%s\n", ERROR_NO_ACTIV);
        return -1;
    }

    /* changing or assigning an user */
    strcpy(saveTaskInfo[taskId].taskUser, userDesc);

    /* beginning the task */
    if (saveTaskInfo[taskId].taskActivityId == ACTIVITY1_ID) {
        saveTaskInfo[taskId].instant = time;
    }

    /* changing the activity */
    saveTaskInfo[taskId].taskActivityId = activityId;

    if (activityId == ACTIVITY3_ID) {
    duration = time - saveTaskInfo[taskId].instant;
    slack = duration - saveTaskInfo[taskId].duration;
    printf("duration=%d slack=%d\n", duration, slack);
    }
    return 0;
}

/* lists all tasks in a certain activity*/
int lists_activity_tasks(char args[])
{
    int scanned = 0, activityId, i, counter = 0, v, w;
    char descActiv[MAX_DESC_ACTIV + 1];
    substring(args, scanned, MAX_DESC_ACTIV + 1, descActiv);

    /* validating the activity description */
    for (activityId = 1; activityId < nextActivityId; activityId++) {
        if (strcmp(saveActivityInfo[activityId].activityDescription, descActiv) == 0) {
            break;
        }
    }
    if (activityId == nextActivityId) {
        printf("%s\n", ERROR_NO_ACTIV);
        return -1;
    }

    /* ordered by the instant they left the activity TO DO */
    /* if more than one has the same instant we shall order those alphabetically*/

    for (i = 1; i < nextTaskId; i++) /* from 1 till the last */ {
        if (strcmp(saveActivityInfo[saveTaskInfo[i].taskActivityId].activityDescription, descActiv) == 0) {
            intArray[counter] = saveTaskInfo[i].taskId;
            counter++;
        }
    }
    list_instant(0, counter);

    for (i = 0; i < counter; i++)
    {
        if (saveTaskInfo[intArray[i]].instant == saveTaskInfo[intArray[i+1]].instant) {
            if (strcmp(saveTaskInfo[intArray[i]].taskDescription, saveTaskInfo[intArray[i+1]].taskDescription) > 0) {
                v = intArray[i];
                w = intArray[i+1];
                intArray[i] = w;
                intArray[i+1] = v;
            }
        }
    }

    for (i = 0; i < counter; i++) {
        printf("%d %d %s\n", saveTaskInfo[intArray[i]].taskId, saveTaskInfo[intArray[i]].instant, saveTaskInfo[intArray[i]].taskDescription);
    }

    return 0;
}

/* adds a new activity or lists all of them*/
int add_list_activity(char args[])
{
    char descActiv[MAX_DESC_ACTIV + 1];
    int i = 0, scanned = 0, vazio;
    
    substring(args, scanned, MAX_DESC_ACTIV + 1, descActiv);
    vazio = strcmp(descActiv, "");

    /* if descActiv isn't empty we want to create a new activity */
    if (vazio != 0) {
        for (i=1; i < nextActivityId; i++) {
            if ((strcmp(saveActivityInfo[i].activityDescription, descActiv) == 0)) {
                printf("%s\n", ERROR_DUP_ACTIV);
                return -1;
            }
        }

        for (i = 0; i != '\n' && i < MAX_DESC_ACTIV; i++) {
            if (descActiv[i] >= 'a' && descActiv[i] <= 'z') {
                printf("%s\n", ERROR_INV_DESC);
                return -1;
            }
        }

        /* see if we can create a new activity (max 10) */
        if (nextActivityId > MAX_ACTIVS) {
            printf("%s\n", ERROR_MANY_ACTIV);
            return -1;
        }

        /* no error found, we can create a new user */
        strcpy(saveActivityInfo[nextActivityId].activityDescription, descActiv);
        nextActivityId++;
        return 0;

    } else { /* we want to list all activities */
        for (i= 1; i < nextActivityId; i++) {
        /* by creation order, 1-10 */
            printf("%s\n", saveActivityInfo[i].activityDescription);
        }
    }  
    return 0;
}