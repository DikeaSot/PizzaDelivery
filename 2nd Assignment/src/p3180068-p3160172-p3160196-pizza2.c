// κάνω import το header file και την ώρα
#include "p3180068-p3160172-p3160196-pizza2.h"
#include <time.h>

// αρχικοποίηση global μεταβλητών
unsigned int seed;
int occCooks = 0; //occupied Cooks
int occOvens = 0; //occupied Ovens
int occDeliv = 0; //occupied Delivery bois
long int total_time_1 = 0;
long int total_time_2 = 0;
long int total_time_3 = 0;

long int max_time_1 = 0;
long int max_time_2 = 0;
long int max_time_3 = 0;
// δήλωση των mutex
pthread_mutex_t lock;
pthread_cond_t cond;
// συνάρτηση όπου γινεται η παραλαβή της παραγγελίας, η προετοιμασία, το ψήσιμο και η παράδοση
void* pizza_order(void *threadId){

	int *tid;
	long int diff_1, diff_2, diff_3;
	/* 
	diff_1: χρόνος προετοιμασίας και ψησίματος
	diff_2: χρόνος που οι πίτσες είναι εκτός φούρνου
	diff_3: πόσος χρόνος χρειάστηκε για να ολοκληρωθεί η παραγγελία
	*/
	tid = (int *)threadId;
	int rc;

	struct timespec ordTime_recv,ordTime_baked,ordTime_deliv;

	//ήρθε μια παραγγελία
	clock_gettime(CLOCK_REALTIME,&ordTime_recv);
	//τυχαίος αριθμός πιτσών
	int rand_num_pizza = min_num_pizza + rand_r(&seed) % max_num_pizza;


	rc = pthread_mutex_lock(&lock);
	//potential error check
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(&rc);
	}

	//όταν το occCooks που αρχικοποιείται με τιμή 0, γίνει ίσο με τον αριθμό των cooks που εργάζονται στην πιτσαρία με βάση την εκφώνηση, τότε βγάλε μηνύμα αναμονής και κάνε lock μέχρι κάποιος cook να είναι ελεύθερος
	while(occCooks == num_cooks){
		printf("Thread %d: All cooks are occupied...\n", *tid);
		rc = pthread_cond_wait(&cond, &lock);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}
	}

	//όταν φτάσω εδώ μία προετοιμασία ξεκινά και δεσμεύω έναν cook
	++occCooks; 
	printf("Thread %d: Preparing %d pizza(s)!\n", *tid, rand_num_pizza);
	rc = pthread_mutex_unlock(&lock);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(&rc);
	}

	//ξεκινάει ένα χρονικό διάστημα sleep για την παρασκευή των πιτσών
	sleep(time_prep*rand_num_pizza);
	printf("Thread %d: Preparation is done, searching for oven!\n", *tid);

	rc = pthread_mutex_lock(&lock);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(&rc);
	}

	// αντίστοιχα με τους cooks, όταν όλοι οι φούρνοι χρησιμοποιούνται κάνω lock και περιμένω μέχρι κάποιος να αποδεσμευτεί
	while(occOvens == num_ovens){
		printf("Thread %d: All ovens are occupied...\n", *tid);
		rc = pthread_cond_wait(&cond, &lock);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}
	}

	// πλέον μία πίτσα μπήκε στο φούρνο και επομένως αυξάνουμε τον αριθμό των κατειλημμένων φουρνων και ο μάγειρας είναι ετοιμος να αναλάβει την επόμενη παραγγελία

	++occOvens;
	--occCooks;
	
	rc = pthread_cond_signal(&cond);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		pthread_exit(&rc);
	}

	printf("Thread %d: Baking %d pizza(s)!\n",*tid,rand_num_pizza);
	rc = pthread_mutex_unlock(&lock);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(&rc);
	}

	//η διαδικασία "κοιμάται" για σταθερό χρόνο time bake
	sleep(time_bake);

	rc = pthread_mutex_lock(&lock);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(&rc);
	}

	//η προετοιμασία της παραγγελίας ολοκληρώθηκε
	clock_gettime(CLOCK_REALTIME,&ordTime_baked);
	
	//συνολικός χρόνος προετοιμασίας και ψησίματος της παραγγελίας
	diff_1 = ordTime_baked.tv_sec - ordTime_recv.tv_sec;
	total_time_1 += diff_1;

	//μέγιστος χρόνος που έκανε μία παραγγελία για να ολοκληρωθεί από τον cook
	if(max_time_1< diff_1){
		max_time_1 = diff_1;
	}

	printf("The order %d was prepared and baked in %ld minutes!\n",*tid, diff_1);

	//εδώ πλέον η παραγγελία παραδόθηκε και απελευθερώνετεαι ο oven που την υλοποίησε
	--occOvens; 

	//unlock mutex 
	rc = pthread_mutex_unlock(&lock);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(&rc);
	}
	
	// η signal κάνει unblock το νήμα που μπλόκαρε
	rc = pthread_cond_signal(&cond);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		pthread_exit(&rc);
	}

	//τυχαίος χρόνος παράδοσης
	int newTime = time_high - time_low +1; 
	int deliveryTime = time_low + rand_r(&seed) % newTime;

	rc = pthread_mutex_lock(&lock);
	//potential error check
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(&rc);
	}

	//όπως και με τους cooks και τους ovens, όταν οι όλοι οι διανομείς είναι δεσμευμένοι, τότε βγάλε μηνύμα αναμονής και κάνε lock μέχρι κάποιος διανομέας να είναι ελεύθερος
	while(occDeliv == num_deliver){

		printf("Thread %d: All delivery bois are occupied...\n", *tid);
		rc = pthread_cond_wait(&cond, &lock);
		if (rc != 0) {	
			printf("ERROR: return code from pthread_cond_wait() is %d\n", rc);
			pthread_exit(&rc);
		}
	}

	++occDeliv;
	printf("Thread %d: Delivering %d pizza(s), it will take me approximately %d minutes\n", *tid, rand_num_pizza, deliveryTime);
	rc = pthread_mutex_unlock(&lock);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(&rc);
	}

	//η διεργασία "κοιμάται" εως ότου παραδωθεί η πίτσα
	sleep(deliveryTime);

	clock_gettime(CLOCK_REALTIME,&ordTime_deliv);

	//ο χρόνος που η παραγγελία κρύωνε
	diff_2 = ordTime_deliv.tv_sec - ordTime_baked.tv_sec;
	total_time_2 += diff_2;

	//μέγιστος χρόνος που έκανε κάποια παραγγελία για να κρυώσει
	if(max_time_2< diff_2){
		max_time_2 = diff_2;
	}

	//συνολικός χρόνος της παραγγελίας μέχρι και την παράδοση της
	diff_3 = ordTime_deliv.tv_sec - ordTime_recv.tv_sec;
	total_time_3 += diff_3;

	//μέγιστος χρόνος που έκανε κάποια παραγγελία για να παραδοθεί
	if(max_time_3< diff_3){
		max_time_3 = diff_3;
	}

	printf("The order %d was delivered in %ld minutes and was cold for %ld!\n",*tid, diff_3,diff_2);

	//περιμένουμε να επιστρέψει πίσω ο διανομέας
	sleep(deliveryTime);

	rc = pthread_mutex_lock(&lock);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rc);
		pthread_exit(&rc);
	}

	//ο διανομέας επέστρεψε, οπότε και γίνεται πάλι διαθέσιμος
	--occDeliv;

	rc = pthread_mutex_unlock(&lock);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_mutex_unlock() is %d\n", rc);
		pthread_exit(&rc);
	}

	rc = pthread_cond_signal(&cond);
	if (rc != 0) {	
		printf("ERROR: return code from pthread_cond_signal() is %d\n", rc);
		pthread_exit(&rc);
	}

	// το thread τελείωσε τη διαδικασία και ακολουθεί η έξοδος του
	pthread_exit(tid);
}



// η Main του προγράμματος
int main (int argc, char *argv[]){

	// error check αν τα arguments δεν είναι σωστά
	if (argc != 3){
		printf("ERROR: the program should take two arguments, the seed and the amount of customers\n");
		exit(-1);
	}

	// αρχικοποίηση μεταβλητών της main
	int threadCount;
	unsigned int customers = 0;
	int nextOrder;
	pthread_t *threads;
	int rc;

	//αρχικοποίηση του seed και των customers
	seed = atoi(argv[1]);
	customers = atoi(argv[2]);

	printf("The seed value is %d\n", seed);
	printf("The amount of customers is %d\n", customers);

	// ο χώρος που καταλαμβάνουν τα threads στη μνήμη
	threads = malloc(customers * sizeof(pthread_t));
	if (threads == NULL) {
		printf("NOT ENOUGH MEMORY!\n");
		return -1;
	}

	//This is the threadID
	int countArray[customers];

	// initialization of mutex
	rc = pthread_mutex_init(&lock, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_mutex_init() is %d\n", rc);
       		exit(-1);
	}

	// initialize the condition variable
  	rc = pthread_cond_init(&cond, NULL);
	if (rc != 0) {
    		printf("ERROR: return code from pthread_cond_init() is %d\n", rc);
       		exit(-1);
	}

	// ο χρόνος που περιμένουμε μέχρι να δεχθούμε την επόμενη παραγγελία
	for(threadCount = 0; threadCount< customers; ++threadCount){

		nextOrder = time_order_min + rand_r(&seed) % time_order_max;
		countArray[threadCount] = threadCount + 1;

		printf("Main: creating thread %d\n", countArray[threadCount]);

		// δημιουργία των threads
		rc =  pthread_create(&threads[threadCount], NULL, pizza_order ,&countArray[threadCount]);
		if (rc != 0) {
			printf("ERROR: return code from pthread_create() is %d\n", rc);
			exit(-1);
		}

		sleep(nextOrder);
	}

	void *status;
	// απαρίθμηση των threads
	for (threadCount = 0; threadCount < customers; ++threadCount) {

		//join threads
		rc = pthread_join(threads[threadCount], &status);
		if(rc != 0){
			printf("ERROR: return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
	}

	printf("The average cooling time was: %f minutes.\n", (double)total_time_2/customers);
	printf("The maximum cooling time was : %ld minutes\n", max_time_2);
	printf("The average delivery time was: %f minutes.\n", (double)total_time_3/customers);
	printf("The maximum delivery time was: %ld minutes\n", max_time_3);

	// πλέον τα mutexes δεν χρειάζονται όποτε τα καταστρέφουμε για να αδειάσει ο χώρος στη μνήμη
	rc = pthread_mutex_destroy(&lock);
	if (rc != 0) {
		printf("ERROR: return code from pthread_mutex_destroy() is %d\n", rc);
		exit(-1);		
	}

 	rc = pthread_cond_destroy(&cond);
	if (rc != 0) {
		printf("ERROR: return code from pthread_cond_destroy() is %d\n", rc);
		exit(-1);		
	}
	// απελευθερώνουμε τα threads αφού ολοκληρώθηκε η διαδικασία
	free(threads);
}
