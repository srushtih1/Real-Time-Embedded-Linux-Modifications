#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TASKS 10000
#define MAX_CPU 100

typedef struct {
    char name[21];
    int C;
    int T;
    float utilization;
	int bin_no;
} Task;

int cmp(const void *a, const void *b) {
    Task *task1 = (Task *)a;
    Task *task2 = (Task *)b;
    if (task1->utilization < task2->utilization) {
        return 1;
    } else {
        return -1;
    }
}


int BFW(Task tasks[], int n_tasks, float bins[], int n_bins) {
    
	qsort(tasks, n_tasks, sizeof(Task), cmp);
    int i, j;
    
	for (i = 0; i < n_tasks; i++) {
        int bin_idx = 0;
		float utilization_to_fit = tasks[i].utilization;
        for (j = 0; j < n_bins; j++) {
            if (bins[j] > bins[bin_idx] || bins[bin_idx]+utilization_to_fit > 1.0) {
                if(bins[j]+utilization_to_fit <= 1.0){
					bin_idx=j;
				}
            }
        }
        if (bins[bin_idx] + tasks[i].utilization <= 1.0) {
            bins[bin_idx] += tasks[i].utilization;
			tasks[i].bin_no = bin_idx;
        }else{
			return -1;
		}
    }
	return 0;
}

int WFD(Task tasks[], int n_tasks, float bins[], int n_bins) {
    qsort(tasks, n_tasks, sizeof(Task), cmp);
    int i, j;
    
	for (i = 0; i < n_tasks; i++) {
        int bin_idx = 0;
        for (j = 0; j < n_bins; j++) {
            if (bins[j] < bins[bin_idx]) {
                bin_idx = j;
            }
        }
        if (bins[bin_idx] + tasks[i].utilization <= 1.0) {
            bins[bin_idx] += tasks[i].utilization;
			tasks[i].bin_no = bin_idx;
        }else{
			return -1;
		}
    }
	return 0;
}

int FFD(Task tasks[], int n_tasks, float bins[], int n_bins) {

	qsort(tasks, n_tasks, sizeof(Task), cmp);
    int i, j;
    
	for (i = 0; i < n_tasks; i++) {
		
        for (j = 0; j < n_bins; j++) {
            if (bins[j] + tasks[i].utilization <= 1.0) {
                bins[j] += tasks[i].utilization;
				tasks[i].bin_no = j;

                break;
            }
        }
		if(tasks[i].bin_no==-1) return -1;
		
    }
	return 0;
}

int main() {
    int n_bins, n_tasks;
    char heuristic[4];
    Task tasks[MAX_TASKS];
    scanf("%d,%3s\n", &n_bins, heuristic);
	if(n_bins>MAX_CPU){
		printf("Failure\n");
		return 0;
	}
    scanf("%d\n", &n_tasks);
	if(n_tasks > MAX_TASKS){
		printf("Failure\n");
		return 0;
	}
	int i, j;
	for (i = 0; i < n_tasks; i++) {
   	 scanf("%20[^,],%d,%d\n", tasks[i].name, &tasks[i].C, &tasks[i].T);

    	tasks[i].utilization = ((float)tasks[i].C)/((float)tasks[i].T);

   	 tasks[i].bin_no = -1;
	}

    float bins[n_bins];
    memset(bins, 0, sizeof(bins));
    if (strcmp(heuristic, "BFW") == 0) {
        if(BFW(tasks, n_tasks, bins, n_bins)==-1) {
			printf("Failure\n");
			return 0;
		}
    }
    else if(strcmp(heuristic, "WFD") == 0) {
        if(WFD(tasks, n_tasks, bins, n_bins)==-1){
			printf("Failure\n");
			return 0;
		}
    }
    else if(strcmp(heuristic, "FFD") == 0) {
        if(FFD(tasks, n_tasks, bins, n_bins)==-1){
			printf("Failure\n");
			return 0;
		}
    }else{
    	printf("Failure\n");	
    }
	printf("Success\n");
	
	for(i=0;i<n_bins;i++){	
		printf("CPU%d", (i+1));
		for(j=0;j<n_tasks;j++){
			if(tasks[j].bin_no == i){
				printf(",%s", tasks[j].name);
			}
		}
		printf("\n");
	}
	return 0;
	
}
