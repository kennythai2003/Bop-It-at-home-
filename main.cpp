/*        Your Name & E-mail: Kenny Thai & kthai025@ucr.edu

*          Discussion Section: 21

 *         Assignment: Final Project Part 3

 *         Exercise Description: the final bop it (at home)
 *        

 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.

 *

 *         Demo Link: https://youtu.be/Cod0daSMnFg

 */

// libraries
#include "serialATmega.h"
#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include "LCD.h"
#include "stdlib.h"
#include "DHT.h"
#include "DHT.c"
#include "irAVR.h" 

// making a queue
#define MAX 100  // Maximum size of the queue

typedef struct Queue {
    int front, rear, size;
    unsigned capacity;
    int* array;
} Queue;

Queue* createQueue(unsigned capacity) {
    Queue* queue = (Queue*) malloc(sizeof(Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;  // Rear starts at the end
    queue->array = (int*) malloc(queue->capacity * sizeof(int));
    return queue;
}

int isFull(Queue* queue) {
    return (queue->size == queue->capacity);
}

int isEmpty(Queue* queue) {
    return (queue->size == 0);
}

void enqueue(Queue* queue, int item) {
    if (isFull(queue)) {
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

int dequeue(Queue* queue) {
    if (isEmpty(queue)) {
        return -1;
    }
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

int front(Queue* queue) {
    if (isEmpty(queue)) {
        return -1;
    }
    return queue->array[queue->front];
}

int rear(Queue* queue) {
    if (isFull(queue)) {
        return -1;
    }
    return queue->array[queue->rear];
}


//TODO: declare variables for cross-task communication
unsigned char time_counter = 0;
unsigned char display_counter = 0;
unsigned char difficulty = 51; // increases depending on score 
unsigned char score = 0; // used to display the score 
static char buffer[10];
static char *score_string = itoa(score, buffer, 10);
unsigned char rand_action;
unsigned char rand_counter = 0; // iterates the srand
bool system_on = 0;
bool val_check = 0;
unsigned char *temperature;
unsigned char *humidity;
unsigned char curr_temp;
unsigned char prev_temp;
unsigned char counter = 0;
Queue* actions_queue = createQueue(2);
decode_results results;
bool actions[7];
    // 0 : bop it
    // 1: joystick up
    // 2: joystick down
    // 3: joystick left
    // 4: joystick right
    // 5: blow
    // 6: cover it 

/* You have 5 tasks to implement for this lab */
#define NUM_TASKS 5

//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long GCD_PERIOD = /* TODO: Calulate GCD of tasks */ 10;

task tasks[NUM_TASKS]; // declared task array with 5 tasks

//TODO: Define, for each task:
// (1) enums and
// (2) tick functions
void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}

// task 0 
enum poweronoff_states {init0, power_on, power_off};
int poweronoff_tick(int state){
    unsigned char remote_value;
    switch (state){
        case init0:
            state = power_off;
            break;
        case power_off:
            if(IRdecode(&results) == 1) {
                if(results.value == 93){
                    state = power_off;
                    IRresume();
                }
                else{
                    state = power_on;
                    IRresume();
                }
            }
            break;
        case power_on:
            remote_value = 0;
            if(IRdecode(&results) == 1) {
                if(results.value == 93){
                    state = power_on;
                    IRresume();
                }
                else{
                    state = power_off;
                    IRresume();
                }
            }
            break;
    }
    switch (state){
        case init0:
            system_on = 0;
            break;
        case power_on:
            system_on = 1;
            break;
        case power_off:
            system_on = 0; 
            break;
    }
    return state;
}   


enum joystick_states {init1, joystick_idle, joystick_press_hold,joystick_up_hold, joystick_down_hold, joystick_left_hold, joystick_right_hold,
                        joystick_press_release, joystick_up_release, joystick_down_release, joystick_left_release, joystick_right_release};
int joystick_tick(int state){
    switch (state){
        case init1:
            state = joystick_idle;
            break;
        case joystick_idle:
            if(!((PINC >> 2) & 0x01)){
                state = joystick_press_hold;
            }
            else if (ADC_read(0) > 980){
                state = joystick_up_hold;
            }
            else if (ADC_read(0) < 300) {
                state = joystick_down_hold;
            }
            else if (ADC_read(1) < 300){
              state = joystick_left_hold;
            }
            else if (ADC_read(1) > 980){
              state = joystick_right_hold;
            }
            else{
                state = joystick_idle;
            }
            break;

        case joystick_press_hold:
            if(!((PINC >> 2) & 0x01)){
                state = joystick_press_hold;
            }
            else{
                state = joystick_press_release;
            }
            break;
        case joystick_press_release:
            if (val_check){
                enqueue(actions_queue, 0);
            }
            state = joystick_idle;
            break;
        case joystick_up_hold:
            if (ADC_read(0) > 980){
                state = joystick_up_hold;
            }
            else {
                state = joystick_up_release;
            }
            break;
        case joystick_down_hold:
            if (ADC_read(0) < 300) {
                state = joystick_down_hold;
            }
            else {
                state = joystick_down_release;
            }
            break;
        case joystick_left_hold:
             if (ADC_read(1) < 300){
              state = joystick_left_hold;
            }
            else {
                state = joystick_left_release;
            }
            break;
        case joystick_right_hold:
             if (ADC_read(1) > 980){
              state = joystick_right_hold;
            }
            else {
                state = joystick_right_release;
            }
            break;
        case joystick_up_release:
            if (val_check){
                enqueue(actions_queue, 1);
            }
            state = joystick_idle;
            break;
        case joystick_down_release:
            if(val_check){
                enqueue(actions_queue, 2);
            }
            state = joystick_idle;
            break;
        case joystick_left_release:
            if(val_check){
                enqueue(actions_queue, 3);
            }
            state = joystick_idle;
            break;
        case joystick_right_release:
            if(val_check){
                enqueue(act ions_queue, 4);
            }
            state = joystick_idle;
            break;
    }
    switch (state){
        case init1:
            break;
        case joystick_idle:
            break;
        case joystick_up_hold:
            break;
        case joystick_down_hold:
            break;
        case joystick_left_hold:
            break;
        case joystick_right_hold:
            break;
        case joystick_up_release:
            break;
        case joystick_down_release:
            break;
        case joystick_left_release:
            break;
        case joystick_right_release:
            break;
        case joystick_press_hold:
            break;
        case joystick_press_release:
            break;
    }
    return state;
}


enum dht_states {init2, dht_sample};
int dht_tick(int state){
    switch (state){
        case init2:
            state = dht_sample;
            break;
        case dht_sample:
            dht_GetTemp(temperature, humidity);
            curr_temp = *temperature;
            if (counter == 0) {
                prev_temp = curr_temp;
            } else if (curr_temp - prev_temp > 2 && val_check) {
                    enqueue(actions_queue, 5);
            }
            counter = (counter + 1) % 4; 
            break;
    }
    switch (state){
        case init2:
            break;
        case dht_sample:
            break;
    }
    return state;
}

enum pir_states {init3, pir_sample};
int pir_tick(int state){
    switch (state){
        case init3:
            state = pir_sample;
            break;
        case pir_sample:
            if (GetBit(PINB, 4) == 1 && val_check) {
                    enqueue(actions_queue, 6);
            }
            break;
        }
    switch (state){
        case init3:
            break;
        case pir_sample:
            break;
    }
    return state;
}

enum game{init4, off, begin, generate, validate, success, display_score, fail};
int game_tick(int state){
    switch(state){
        case init4:
            state = off;
            break;
        case off:
            if (system_on){
                state = begin;
            }
            else {
                state = off;
            }
            break;
        case begin:
            display_counter += 1;
            if (display_counter  == 1){
                lcd_clear();
                lcd_write_str("Starting...");
            }
            else if (display_counter == 20) {
                display_counter = 0;
                state = generate;
            }
            break;
        case generate:
            srand(rand_counter);
            rand_counter++;
            rand_action = rand() % 7;
            actions[rand_action] = 1;
            if (actions[0]){
                lcd_clear();
                lcd_write_str("Action: Bop It");
                lcd_goto_xy(1, 0);
                lcd_write_str("Time: 0");
            }
            else if (actions[1]){
                lcd_clear();
                lcd_write_str("Action: Up");
                lcd_goto_xy(1, 0);
                lcd_write_str("Time: 0");
            }
            else if (actions[2]){
                lcd_clear();
                lcd_write_str("Action: Down");
                lcd_goto_xy(1, 0);
                lcd_write_str("Time: 0");
            }
            else if (actions[3]){
                lcd_clear();
                lcd_write_str("Action: Left");
                lcd_goto_xy(1, 0);
                lcd_write_str("Time: 0");
            }
            else if (actions[4]){
                lcd_clear();
                lcd_write_str("Action: Right");
                lcd_goto_xy(1, 0);
                lcd_write_str("Time: 0");
            }
            else if (actions[5]){
                lcd_clear();
                lcd_write_str("Action: Blow");
                lcd_goto_xy(1, 0);
                lcd_write_str("Time: 0");
            }
            else if (actions[6]){
                lcd_clear();
                lcd_write_str("Action: Hover");
                lcd_goto_xy(1, 0);
                lcd_write_str("Time: 0");
            }
            state = validate;
            break;
        case success:
            display_counter += 1;
            if (display_counter  == 1){
                lcd_clear();
                lcd_write_str("Success");
                lcd_goto_xy(1, 0);
                lcd_write_str("Score: ");
                score_string = itoa(score, buffer, 10);
                lcd_write_str(score_string);
            }
            else if (display_counter == 10) {
                display_counter = 0;
                state = generate;
            }
            break;
        case validate:
            if (time_counter < difficulty){
                if (!isEmpty(actions_queue)){
                    if (actions[front(actions_queue)] == 1) {
                        dequeue(actions_queue);
                        actions[rand_action] = 0;
                        score++;
                        state = success;
                    }
                    else {
                        dequeue(actions_queue);
                        actions[rand_action] = 0;
                        state = display_score;
                    }
                }
                if ((time_counter % 10) == 0) {
                    lcd_goto_xy(1, 0);
                    lcd_write_str("Time: ");
                    lcd_write_str(itoa(time_counter/10, buffer, 10));
                }
            }
            else {
                state = display_score;
            }
            break;
        case display_score:
            display_counter += 1;
            if (display_counter  == 1){
                lcd_clear();
                lcd_write_str("Fail!");
                lcd_goto_xy(1, 0);
                lcd_write_str("Final Score: ");
                score_string = itoa(score, buffer, 10);
                lcd_write_str(score_string);
            }
            else if (display_counter == 10) {
                display_counter = 0;
                state = fail;
            }
            break;
        case fail:
            display_counter += 1;
            if (display_counter  == 1){
                lcd_clear();
                lcd_write_str("Press Power to ");
                lcd_goto_xy(1, 0);
                lcd_write_str("Play Again");
            }
            else if (display_counter == 10) {
                display_counter = 0;
            }
            if (system_on){
                display_counter = 0;
                state = begin;
            }
            else {
                state = fail;
            }
            break;
    }
    switch(state){
       case init4:
            break;
        case off:
            break;
        case begin:
            break;
        case generate:
            break;
        case success:
            time_counter = 0;
            val_check = 0;
            if (score == 3){
                difficulty = 31;
            }
            else if (score == 6){
                difficulty = 21;
            }
            break;
        case validate:
            time_counter++;
            val_check = 1;
            break;
        case display_score:
            break;
        case fail:
            score = 0;
            difficulty = 51;
            time_counter = 0;
            val_check = 0;
            break;
    }
    return state;
}

int main(void) {
    
    ADC_init();   // initializes ADC
    lcd_init(); // initializes lcd
    serial_init(9600);
    
    //TODO: initialize all your inputs and ouputs
    // DDRB = 0x00;
    // PORTB = 0xFF;
    DDRB = 0xCE;
    PORTB = 0x31;

    DDRC = 0xF8;
    PORTC = 0x07;

    DDRD = 0xFF;
    PORTD = 0x00;


    tasks[0].period = 50;
    tasks[0].state = init0;
    tasks[0].elapsedTime = 50;
    tasks[0].TickFct = &poweronoff_tick;

    tasks[1].period = 10;
    tasks[1].state = init1;
    tasks[1].elapsedTime = 10;
    tasks[1].TickFct = &joystick_tick;
    
    tasks[2].period = 100;
    tasks[2].state = init2;
    tasks[2].elapsedTime = 100;
    tasks[2].TickFct = &dht_tick;

    tasks[3].period = 100;
    tasks[3].state = init3;
    tasks[3].elapsedTime = 100;
    tasks[3].TickFct = &pir_tick;
    
    tasks[4].period = 100;
    tasks[4].state = init4;
    tasks[4].elapsedTime = 100;
    tasks[4].TickFct = &game_tick;

    TimerSet(GCD_PERIOD);
    TimerOn();
    lcd_clear();

    lcd_write_str("Bop It at Home");
    lcd_goto_xy(1, 0);
    lcd_write_str("Power to Start");
    
    IRinit(&PORTB, &PINB, 5);
    while (1) {
        
    }

    
    return 0;
}