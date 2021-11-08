/*
 * setitimer.c - simple use of the interval timer
 */

#include <sys/time.h>           /* for setitimer */
#include <unistd.h>             /* for pause */
#include <signal.h>             /* for signal */
#include <stdlib.h>
#include <stdio.h>
#include <gpiod.h>



#ifndef CONSUMER
#define CONSUMER        "Consumer"
#endif

#define INTERVAL 500            /* number of milliseconds to go off */


char *chipname = "gpiochip0";
unsigned int LED_GPIO_NUM = 27;
unsigned int BUTTON_GPIO_NUM = 26;
unsigned int val=0;
struct gpiod_chip *chip;
struct gpiod_line *LED_GPIO;
struct gpiod_line *BUTTON_GPIO;
int i, ret;


struct itimerval it_val;      /* for setting itimer */

struct timeval stop,start,elapsed;
double newt = 0.0;
int flag = 0; 
/* function prototype */
void DoStuff(void);

int main(int argc, char *argv[]) {

  // OPEN GPIO CHIP
  chip = gpiod_chip_open_by_name(chipname);
  if (!chip) {
    perror("Open chip failed\n");
        goto end;

  }
  // OPEN GPIO LINES
  LED_GPIO = gpiod_chip_get_line(chip, LED_GPIO_NUM);
  if (!LED_GPIO) {
               perror("Get line failed\n");
                goto close_chip;
  }
  BUTTON_GPIO = gpiod_chip_get_line(chip, BUTTON_GPIO_NUM);
  if (!LED_GPIO) {
                perror("Get line failed\n");
                goto close_chip;
  }
  // OPEN GPIO PIN FOR OUTPUT TO MAKE IT FLASH LED
  ret = gpiod_line_request_output(LED_GPIO, CONSUMER, 0);
        if (ret < 0) {
                perror("Request line as output failed\n");
                goto release_line;
        }
  // OPEN GPIO PIN FOR INPUT TO COUNT HOW LONG BUTTON PRESSED FOR
   ret = gpiod_line_request_input(BUTTON_GPIO, CONSUMER);
        if (ret < 0) {
                perror("Request line as output failed\n");
                goto release_line;
        }

 
  /* Upon SIGALRM, call DoStuff().
   * Set interval timer.  We want frequency in ms, 
  * but the setitimer call needs seconds and useconds. */
  if (signal(SIGALRM, (void (*)(int)) DoStuff) == SIG_ERR) {
    perror("Unable to catch SIGALRM");
    exit(1);
  }
  it_val.it_value.tv_sec =     INTERVAL/1000;
  it_val.it_value.tv_usec =    (INTERVAL*1000) % 1000000;       
  it_val.it_interval = it_val.it_value;
  if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
    perror("error calling setitimer()");
    exit(1);
  }

        while(1){
        // Read from the button
        ret = gpiod_line_get_value(BUTTON_GPIO);
        if (ret < 0) {
        perror("Set line output failed\n");
        gpiod_line_release(BUTTON_GPIO);
        }
        // printf("Press : %d\n", ret);
        // If button being pressed for first time start timer
        if(ret == 1 && flag == 0){
        gettimeofday(&start,NULL);
        flag = 1;
        }
        // If button not being pressed but previously had been then end timer
        else if(ret == 0 && flag == 1){
        flag = 0;
        gettimeofday(&stop,NULL);
        timersub(&stop,&start,&elapsed);
        printf("Button Delay %f\n",elapsed.tv_sec + elapsed.tv_usec/1000000.0);
        printf("sec %d\n" , elapsed.tv_sec);
        printf("usec %f\n",elapsed.tv_usec/1000000.0);

        it_val.it_value.tv_sec  = elapsed.tv_sec ;
        it_val.it_value.tv_usec = elapsed.tv_usec;
        it_val.it_interval = it_val.it_value;

        // Set the new timer for LED duration
        if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
        perror("error calling setitimer()");
        exit(1);
        }
        }

        }

        release_line:
        gpiod_line_release(LED_GPIO);
        gpiod_line_release(BUTTON_GPIO);
        close_chip:
        gpiod_chip_close(chip);
        end:
        return 0;

        }

/*
 * DoStuff
 */
void DoStuff(void) {

        // Toggle the LED
        ret = gpiod_line_set_value(LED_GPIO, val%2);
        if (ret < 0) {
        perror("Set line output failed\n");
        gpiod_line_release(LED_GPIO);
        }
        // Make sure LED toggles next time
        val +=1;

        // Read from the button
        ret = gpiod_line_get_value(BUTTON_GPIO);
        if (ret < 0) {
        perror("Set line output failed\n");
        gpiod_line_release(BUTTON_GPIO);
        }
        //printf("Press : %d\n", ret);
        // If button being pressed for first time start timer
        if(ret == 1 && flag == 0){
        gettimeofday(&start,NULL);
        flag = 1;
        }
        // If button not being pressed but previously had been then end timer
        else if(ret == 0 && flag == 1){
        flag = 0;
        gettimeofday(&stop,NULL);
        timersub(&stop,&start,&elapsed);
        printf("Button Delay %f\n",elapsed.tv_sec + elapsed.tv_usec/1000000.0);
        printf("sec %d\n" , elapsed.tv_sec);
        printf("usec %f\n",elapsed.tv_usec/1000000.0);

        it_val.it_value.tv_sec  = elapsed.tv_sec ;
        it_val.it_value.tv_usec = elapsed.tv_usec;
        it_val.it_interval = it_val.it_value;

        // Set the new timer for LED duration
        if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
        perror("error calling setitimer()");
        exit(1);
        }
        }
}

