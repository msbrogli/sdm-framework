
#include <stdio.h>
#include "sdm.h"

int main(void) {
	struct counter_s counter;
	counter_create_file("test4_counter", 1000, 1000000);

	counter_init_file("test4_counter", &counter);
	counter_print_summary(&counter);
}

