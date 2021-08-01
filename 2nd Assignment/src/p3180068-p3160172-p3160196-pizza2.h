#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

const int num_cooks = 2;
const int num_ovens = 5;
const int num_deliver = 10;
const int time_order_min = 1;
const int time_order_max = 5;
const int min_num_pizza = 1;
const int max_num_pizza = 5;
const int time_prep = 1;
const int time_bake = 10;
const int time_low = 5;
const int time_high = 15;
