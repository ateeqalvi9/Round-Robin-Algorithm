/**
 * PROJECT: Round Robin CPU Scheduler Simulation
 * ARCHITECT: Ahmad Rayan Qasim
 * CONCEPTS: Queue ADT (Manual Linked List), Object Oriented Design, Algorithmic Simulation
 * * NOTE: No <vector>, <queue>, or <list> used. All data structures are manual.
 */

#include <iostream>
#include <iomanip>

using namespace std;


// 1. Domain Object: Process

class Process 
{
public:
    int id;
    int arrivalTime;
    int burstTime;
    int remainingTime;
    int waitingTime;
    int turnaroundTime;
    int completionTime;

    Process(){} 
    Process(int pid, int at, int bt) 
	{
        id = pid;
        arrivalTime = at;
        burstTime = bt;
        remainingTime = bt;
        waitingTime = 0;
        turnaroundTime = 0;
        completionTime = 0;
    }
};


//2. Data Structure: Node (for Linked List)

struct ProcessNode {
    Process data;
    ProcessNode* next;

    ProcessNode(Process p) {
        data = p;
        next = NULL;
    }
};


// 3. Data Structure: Custom Queue (FIFO)
class SchedulerQueue 
{
private:
    ProcessNode* head;
    ProcessNode* tail;

public:
    SchedulerQueue() 
	{
        head = NULL;
        tail = NULL;
    }

    // Destructor to prevent memory leaks
    ~SchedulerQueue() 
	{
        while (!isEmpty()) 
		{
            dequeue();
        }
    }

    bool isEmpty() {
        return head == NULL;
    }

    // O(1) Enqueue - Adds to the end (Tail)
    void enqueue(Process p) {
        ProcessNode* newNode = new ProcessNode(p);
        if (isEmpty()) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    // O(1) Dequeue - Removes from the front (Head)
    Process dequeue() {
        if (isEmpty()) {
            //Should be handled by caller, but returning dummy for safety
            return Process(-1, 0, 0);
        }
        ProcessNode* temp = head;
        Process p = temp->data;
        
        head = head->next;
        if (head == NULL) {
            tail = NULL;
        }
        
        delete temp; //Critical: Manual memory management
        return p;
    }
};


// 4. The Engine: Round Robin Simulator
class RoundRobinSimulator 
{
private:
    Process* processList; //Array to hold initial inputs
    Process* completedList; //Array to hold finished processes for stats
    int processCount;
    int timeQuantum;

    // Helper: Sort processes by Arrival Time (Bubble Sort)
    // Why? To ensure we load them into the simulator in the correct chronological order.
    void sortProcessesByArrival() 
	{
        for (int i = 0; i < processCount - 1; i++) 
		{
            for (int j = 0; j < processCount - i - 1; j++) 
			{
                if (processList[j].arrivalTime > processList[j + 1].arrivalTime) 
				{
                    Process temp = processList[j];
                    processList[j] = processList[j + 1];
                    processList[j + 1] = temp;
                }
            }
        }
    }

public:
    RoundRobinSimulator(int count, int quantum) 
	{
        processCount = count;
        timeQuantum = quantum;
        processList = new Process[count];//Dynamic Array
        completedList = new Process[count];//Dynamic Array
    }

    ~RoundRobinSimulator() 
	{
        delete[] processList;
        delete[] completedList;
    }

    void addProcess(int index, int pid, int at, int bt) 
	{
        processList[index] = Process(pid, at, bt);
    }

    void run() 
	{
        sortProcessesByArrival();

        SchedulerQueue readyQueue;
        int currentTime = 0;
        int completedCount = 0;
        int inputIndex = 0; //Tracks which processes from input list have been added to queue

        cout << "\n=== GANTT CHART (Execution Flow) ===\n";
        cout << "Start ";

        //Simulation Loop
        while (completedCount < processCount) 
		{
            
            //1. Check for new arrivals at this exact 'currentTime'
            // We loop here in case multiple processes arrive at the same time
            while (inputIndex < processCount && processList[inputIndex].arrivalTime <= currentTime) 
			{
                readyQueue.enqueue(processList[inputIndex]);
                inputIndex++;
            }

            if (readyQueue.isEmpty()) 
			{
                // CPU is idle, jump time forward
                if (inputIndex < processCount) 
				{
                    // Jump to the next arrival time to save cycles
                    currentTime = processList[inputIndex].arrivalTime;
                }
                continue; // Restart loop to enqueue the process
            }

            // 2. Dispatch Process
            Process currentP = readyQueue.dequeue();
            
            // 3. Execution Logic
            int timeSlice = 0;
            
            if (currentP.remainingTime > timeQuantum) 
			{
                timeSlice = timeQuantum;
            } 
			
			else 
			{
                timeSlice = currentP.remainingTime;
            }

            // 4. Update Time and State
            // Visualize execution
            cout << "| P" << currentP.id << " (" << currentTime << "-" << currentTime + timeSlice << ") ";
            
            currentTime += timeSlice;
            currentP.remainingTime -= timeSlice;

            // 5. Check again for arrivals that happened *during* this time slice
            while (inputIndex < processCount && processList[inputIndex].arrivalTime <= currentTime) 
			{
                readyQueue.enqueue(processList[inputIndex]);
                inputIndex++;
            }

            // 6. Context Switch Decision
            if (currentP.remainingTime > 0) 
			{
                // Not finished, go back to queue
                readyQueue.enqueue(currentP);
            } 
			else 
			{
                // Process Finished
                currentP.completionTime = currentTime;
                currentP.turnaroundTime = currentP.completionTime - currentP.arrivalTime;
                currentP.waitingTime = currentP.turnaroundTime - currentP.burstTime;
                
                // Store in completion list for final stats
                completedList[completedCount] = currentP;
                completedCount++;
            }
        }
        cout << "| End (" << currentTime << ")\n";
        
        displayMetrics(currentTime);
    }

    void displayMetrics(int totalTime) 
	{
        float totalWait = 0, totalTurnaround = 0, totalBurst = 0;

        cout << "\n\n=== FINAL PERFORMANCE METRICS ===\n";
        cout << "----------------------------------------------------------------------\n";
        cout << "| PID | Arrival | Burst | Completion | Turnaround | Waiting |\n";
        cout << "----------------------------------------------------------------------\n";

        for (int i = 0; i < processCount; i++) 
		{
            Process p = completedList[i];
            totalWait += p.waitingTime;
            totalTurnaround += p.turnaroundTime;
            totalBurst += p.burstTime;

            cout << "| " << setw(3) << p.id << " | " 
                 << setw(7) << p.arrivalTime << " | " 
                 << setw(5) << p.burstTime << " | " 
                 << setw(10) << p.completionTime << " | " 
                 << setw(10) << p.turnaroundTime << " | " 
                 << setw(7) << p.waitingTime << " |\n";
        }
        cout << "----------------------------------------------------------------------\n";

        cout << fixed << setprecision(2);
        cout << "\nAverage Waiting Time:    " << (totalWait / processCount) << " ms";
        cout << "\nAverage Turnaround Time: " << (totalTurnaround / processCount) << " ms";
        
        float cpuUtil = (totalBurst / totalTime) * 100;
        cout << "\nCPU Utilization:         " << cpuUtil << " %\n";
    }
};


// 5. User Interface / Main

int main() {
    int n, quantum;

    cout << "==========================================\n";
    cout << "  ROUND ROBIN SCHEDULER (Manual Queue)    \n";
    cout << "==========================================\n";

    //Input Validation: Process Count
    cout << "Enter Number of Processes: ";
    while(!(cin >> n) || n <= 0) {
        cin.clear(); cin.ignore(1000, '\n');
        cout << "Invalid. Enter a positive integer: ";
    }

    //Input Validation: Time Quantum
    cout << "Enter Time Quantum: ";
    while(!(cin >> quantum) || quantum <= 0) {
        cin.clear(); cin.ignore(1000, '\n');
        cout << "Invalid. Enter a positive integer: ";
    }

    RoundRobinSimulator scheduler(n, quantum);
//Input Validation required
    cout << "\n-- Enter Process Details --\n";
    for (int i = 0; i < n; i++) {
        int pid, at, bt;
        cout << "Process " << (i + 1) << " ID: ";
        cin >> pid;
        cout << "  Arrival Time: ";
        cin >> at;
        cout << "  Burst Time:   ";
        cin >> bt;
        
        //Basic validation for burst time
        if (bt <= 0) bt = 1; 

        scheduler.addProcess(i, pid, at, bt);
        cout << "---------------------------\n";
    }

    scheduler.run();

    return 0;
}
