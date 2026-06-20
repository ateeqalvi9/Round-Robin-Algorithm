#include <iostream>
#include <iomanip>
#include <limits>

using namespace std;

/**
 * PROJECT: Round Robin CPU Scheduler
 * TEAM: Ahmad Rayan Qasim, Attique Alvi
 * * NOTE TO INSTRUCTOR: We avoided all STL containers. 
 * Everything here—from the Queue to the Merge Sort—is implemented manually
 * to show our grip on memory management and O(log N) sorting.
 */


//1. SAFE INPUT UTILITIES (Attique's Part)

int getValidatedInt(const string& prompt, int minValue) 
{
    int value;
    while (true) 
	{
        cout << prompt;
        
        if (!(cin >> value)) 
		{
            cout << "(! Error) Numbers only, please. Resetting input...\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        } 
		
		else if (value < minValue) {
            cout << "(! Error) Value must be at least " << minValue << ". Try again.\n";
        } 
		
		else {
            return value;
        }
    }
}


//2. DOMAIN CLASS (Attique's Part)

class Process {
	
	public:
	    int id, arrivalTime, burstTime, remainingTime;
	    int waitingTime, turnaroundTime, completionTime;
	
	    Process(){
	    	
	        id = arrivalTime = burstTime = remainingTime = 0;
	        waitingTime = turnaroundTime = completionTime = 0;
	    }
	
	    Process(int pid, int at, int bt){
	    	
	        id = pid; arrivalTime = at; burstTime = bt;
	        
	        remainingTime = bt; //Initially, remaining time is just the total burst
	        
	        waitingTime = turnaroundTime = completionTime = 0;
	        
	    }
};


// 3. MANUAL QUEUE ADT (Ahmad's Part)

struct ProcessNode 
{
    Process data;
    ProcessNode* next;
    
    ProcessNode(Process p) : data(p), next(NULL) {}
};

class SchedulerQueue 

{
	private:
	    ProcessNode *head, *tail;
	    
	public:
	    SchedulerQueue() : head(NULL), tail(NULL) {}
	
	    // Destructor: Ahmad added this to make sure we don't leave "ghost" nodes in memory after the program closes
	    ~SchedulerQueue(){	
	        while (!isEmpty()) dequeue();
	        
	    }
	
	    bool isEmpty(){ 
		return head == NULL; }
	
	    //O(1) Enqueue: We use a tail pointer so we don't have to loop through the whole list to add a process
	    void enqueue(Process p){
	    	
	        ProcessNode* node = new ProcessNode(p);
	        
	        if (isEmpty()) head = tail = node;
	        
	        else{
	        	
	            tail->next = node;
	            tail = node;
	        }
	    }
	
	    Process dequeue(){
	    	
	        if (isEmpty()) return Process(-1, 0, 0);
	        
	        ProcessNode* temp = head;
	        Process p = temp->data;
	        head = head->next;
	        
	        if (!head) tail = NULL;
	        
	        delete temp; 
	        return p;
	    }
};


//4. ROUND ROBIN SIMULATOR (Combined Team Effort)

class RoundRobinSimulator 
{
	
	public:
	    Process* processList;
	
	private:
	    Process* completedList;
	    int n, tq;
	
	    //ELITE FEATURE: Manual Merge Sort (Divide & Conquer)
	    //We replaced Bubble Sort with this because O(N log N) is much faster.
	    
	    void merge(Process arr[],int l, int m, int r){
	    	
	        int n1 = m - l + 1;
	        int n2 = r - m;
	        
	        Process* L = new Process[n1];
	        Process* R = new Process[n2];
	
	        for (int i = 0; i < n1; i++) L[i] = arr[l + i];
	        
	        for (int j = 0; j < n2; j++) R[j] = arr[m + 1 + j];
	
	        int i = 0, j = 0, k = l;
	        
	        while (i < n1 && j < n2){
	        	
	            if (L[i].arrivalTime <= R[j].arrivalTime) arr[k++] = L[i++];
	            
	            else arr[k++] = R[j++];
	            
	        }
	        
	        while (i < n1) arr[k++] = L[i++];
	        
	        while (j < n2) arr[k++] = R[j++];
	
	        delete[] L; delete[] R;
	    }
	
	    void mergeSort(Process arr[], int l, int r){
	    	
	        if (l >= r) return;
	        
	        int m = l + (r - l) / 2;
	        
	        mergeSort(arr, l, m);
	        mergeSort(arr, m + 1, r);
	        merge(arr, l, m, r);
	        
	    }
	
	public:
	    RoundRobinSimulator(int count, int quantum) : n(count), tq(quantum){
	    	
	        processList = new Process[n];
	        completedList = new Process[n];
	    }
	
	    ~RoundRobinSimulator(){
	    	
	        delete[] processList;
	        delete[] completedList;
	    }
	
	    void addProcess(int i, int pid, int at, int bt){
	    	
	        processList[i] = Process(pid, at, bt);
	    }
	
	    void run(){
	    	
	        //Step 1: Sort by arrival so the simultor knows who comes first
	        
	        mergeSort(processList, 0, n - 1);
	
	        SchedulerQueue q;
	        int currentTime = 0, inputIndex = 0, completedCount = 0;
	
	        cout << "\n>>> CPU TIMELINE (Gantt Chart)\nStart ";
	
	        while (completedCount < n){
	        	
	            //Check if any process has "arrived" at the current clock time
	            
	            while (inputIndex < n && processList[inputIndex].arrivalTime <= currentTime) {
	            	
	                q.enqueue(processList[inputIndex++]);
	                
	            }
	
	            //If queue is empty, CPU is just sitting there idle
	            if (q.isEmpty()){
	            	
	                if (inputIndex < n) currentTime = processList[inputIndex].arrivalTime;
	                continue;
	            }
	
	            Process p = q.dequeue();
	            
	            // Manual check: Should we run for the full Quantum or just the remaining time?
	            int slice = (p.remainingTime > tq) ? tq : p.remainingTime;
	
	            cout << "-> [P" << p.id << ": " << currentTime << "-" << currentTime + slice << "] ";
	            
	            currentTime += slice;
	            p.remainingTime -= slice;
	
	            //Important: Check for new arrivals DURING the time slice before re-adding current process
	            while (inputIndex < n && processList[inputIndex].arrivalTime <= currentTime){
	                q.enqueue(processList[inputIndex++]);
	            }
	
	            if (p.remainingTime > 0){
	                q.enqueue(p); //Not done yet? Back to the end of the line!
	            } 
				
				else{
					
	                //Process finished! Time to calculate the stats Ateeque worked on
	                p.completionTime = currentTime;
	                p.turnaroundTime = p.completionTime - p.arrivalTime;
	                p.waitingTime = p.turnaroundTime - p.burstTime;
	                completedList[completedCount++] = p;
	                
	            }
	        }
	        cout << " -> End\n";
	        printMetrics(currentTime);
	    }
	
	    void printMetrics(int totalTime){
	    	
	        float totalWT = 0, totalTAT = 0, totalBT = 0;
	        cout << "\n" << setfill('=') << setw(60) << "" << setfill(' ') << endl;
	        
	        cout << left << setw(6) << "PID" << setw(10) << "Arrival" << setw(10) << "Burst" 
	             << setw(10) << "Comp" << setw(10) << "TAT" << "Waiting" << endl;
	             
	        cout << setfill('-') << setw(60) << "" << setfill(' ') << endl;
	
	        for (int i = 0; i < n; i++){
	        	
	            Process p = completedList[i];
	            totalWT += p.waitingTime; totalTAT += p.turnaroundTime; totalBT += p.burstTime;
	            
	            cout << left << setw(6) << p.id << setw(10) << p.arrivalTime << setw(10) << p.burstTime 
	                 << setw(10) << p.completionTime << setw(10) << p.turnaroundTime << p.waitingTime << endl;
	                 
	        }
	
	        cout << setfill('-') << setw(60) << "" << setfill(' ') << endl;
	        cout << fixed << setprecision(2);
	        cout << "Average Waiting Time : " << totalWT / n << " ms\n";
	        cout << "Average Turnaround   : " << totalTAT / n << " ms\n";
	        cout << "CPU Utilization      : " << (totalBT / totalTime) * 100 << "%\n";
	    }
};


//5. MAIN (The Progrm Entry)

int main(){
	
    cout << "------------------------------------------\n";
    cout << "   OS SCHEDULER PROJECT (ROUND ROBIN)     \n";
    cout << "   Developed by: Ahmad & Ateeque \n";
    cout << "------------------------------------------\n";

    int n = getValidatedInt("Enter total processes: ", 1);
    
    int tq = getValidatedInt("Enter Time Quantum (ms): ", 1);

    RoundRobinSimulator engine(n, tq);

    for(int i = 0; i < n; i++){
    	
        cout << "\nData for Process " << i + 1 << ":\n";
        
        int id;
        bool unique;
        
        do {
            unique = true;
            id = getValidatedInt("  - Unique ID: ", 1);
            
            for (int j = 0; j < i; j++){
            	
                if (engine.processList[j].id == id){
                	
                    cout << "(! Error) ID " << id << " is already taken. Try again.\n";
                    unique = false;
                    break;
                    
                }
            }
            
        }while (!unique);

        int at = getValidatedInt("  - Arrival Time: ", 0);
        
        int bt = getValidatedInt("  - Burst Time: ", 1);
        
        engine.addProcess(i, id, at, bt);
        
    }

    engine.run();

    cout << "\nSimulation Complete. Thank you!\n";
    return 0;
}
