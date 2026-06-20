#include <iostream>
#include <iomanip>
#include <limits>

using namespace std;

// SAFE INPUT UTILITIES

int getValidatedInt(const string& prompt, int minValue)
{
    int value;
    while (true)
    {
        cout << prompt;

        if (!(cin >> value))
        {
            cout << "Invalid input. Enter numbers only.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        else if (value < minValue)
        {
            cout << "Value must be >= " << minValue << ". Try again.\n";
        }
        else
            return value;
    }
}

// DOMAIN CLASS

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
	
	    Process()
	    {
	        id = arrivalTime = burstTime = remainingTime = 0;
	        waitingTime = turnaroundTime = completionTime = 0;
	    }
	
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

// LINKED LIST NODE

struct ProcessNode
{
    Process data;
    ProcessNode* next;
    
    ProcessNode(Process p)
    {
        data = p;
        next = NULL;
    }
};

// QUEUE IMPLEMENTATION

class SchedulerQueue
{
	private:
	    ProcessNode* head;
	    ProcessNode* tail;
	
	public:
	
	    SchedulerQueue()
	    {
	        head = tail = NULL;
	    }
	
	    ~SchedulerQueue()
	    {
	        while (!isEmpty())
	            dequeue();
	    }
	
	    bool isEmpty()
	    {
	        return head == NULL;
	    }
	
	    void enqueue(Process p)
	    {
	        ProcessNode* node = new ProcessNode(p);
	        if (isEmpty())
	            head = tail = node;
	        else
	        {
	            tail->next = node;
	            tail = node;
	        }
	    }
	
	    Process dequeue()
	    {
	        if (isEmpty())
	            return Process(-1, 0, 0);
	        ProcessNode* temp = head;
	        Process p = temp->data;
	        head = temp->next;
	        if (!head)
	            tail = NULL;
	        delete temp;
	        return p;
	    }
};

// ROUND ROBIN SIMULATOR

class RoundRobinSimulator
{
	public:
	    Process* processList;
	
	private:
	    Process* completedList;
	    int processCount;
	    int timeQuantum;
	
	private:
	    void sortByArrival()
	    {
	        for(int i=0;i<processCount-1;i++)
	            for(int j=0;j<processCount-1-i;j++)
	                if(processList[j].arrivalTime > processList[j+1].arrivalTime)
	                    swap(processList[j], processList[j+1]);
	    }
	
	public:
	    RoundRobinSimulator(int count, int quantum)
	    {
	        processCount = count;
	        timeQuantum = quantum;
	        processList   = new Process[count];
	        completedList = new Process[count];
	    }
	
	    ~RoundRobinSimulator()
	    {
	        delete[] processList;
	        delete[] completedList;
	    }
	
	    void addProcess(int i, int pid, int at, int bt)
	    {
	        processList[i] = Process(pid, at, bt);
	    }
	
	    void run()
	    {
	        sortByArrival();
	        SchedulerQueue queue;
	        int currentTime = 0;
	        int inputIndex = 0;
	        int completedCount = 0;
	        cout << "\n=== GANTT CHART ===\nStart ";
	        while (completedCount < processCount)
	        {
	            while (inputIndex < processCount && processList[inputIndex].arrivalTime <= currentTime)
	            {
	                queue.enqueue(processList[inputIndex++]);
	            }
	            if (queue.isEmpty())
	            {
	                if (inputIndex < processCount)
	                    currentTime = processList[inputIndex].arrivalTime;
	                continue;
	            }
	            Process p = queue.dequeue();
	            int slice = (p.remainingTime > timeQuantum) ? timeQuantum : p.remainingTime;
	            cout << "| P" << p.id << " (" << currentTime
	                 << "-" << currentTime + slice << ") ";
	            currentTime += slice;
	            p.remainingTime -= slice;
	            while (inputIndex < processCount &&
	                processList[inputIndex].arrivalTime <= currentTime)
	            {
	                queue.enqueue(processList[inputIndex++]);
	            }
	            if (p.remainingTime > 0)
	                queue.enqueue(p);
	            else
	            {
	                p.completionTime = currentTime;
	                p.turnaroundTime = p.completionTime - p.arrivalTime;
	                p.waitingTime    = p.turnaroundTime - p.burstTime;
	
	                completedList[completedCount++] = p;
	            }
	        }
	        cout << "| End (" << currentTime << ")\n";
	        printMetrics(currentTime);
	    }
	
	    void printMetrics(int totalTime)
	    {
	        float totalWT = 0, totalTAT = 0, totalBT = 0;
	        cout << "\n\n=== PROCESS STATS ===\n";
	        cout << "-------------------------------------------------------------\n";
	        cout << "| PID | Arr | Burst | Comp | Turnaround | Waiting |\n";
	        cout << "-------------------------------------------------------------\n";
	        for(int i=0;i<processCount;i++)
	        {
	            Process p = completedList[i];
	            totalWT  += p.waitingTime;
	            totalTAT += p.turnaroundTime;
	            totalBT  += p.burstTime;
	            cout << "| " << setw(3) << p.id
	                 << " | " << setw(3)  << p.arrivalTime
	                 << " | " << setw(5)  << p.burstTime
	                 << " | " << setw(4)  << p.completionTime
	                 << " | " << setw(10) << p.turnaroundTime
	                 << " | " << setw(7)  << p.waitingTime
	                 << " |\n";
	        }
	        float avgWT  = totalWT / processCount;
	        float avgTAT = totalTAT / processCount;
	        cout << "-------------------------------------------------------------\n";
	        cout << fixed << setprecision(2);
	        cout << "Average Waiting Time : " << avgWT << " ms\n";
	        cout << "Average Turnaround   : " << avgTAT << " ms\n";
	        cout << "CPU Utilization      : " << (totalBT / totalTime) * 100 << "%\n";
	    }
};

// HELPER: PID CHECK

bool pidExists(Process* list, int count, int id)
{
    for(int i = 0; i < count; i++)
        if(list[i].id == id)
            return true;

    return false;
}

// MAIN PROGRAM

int main()
{
    cout << "==========================================\n";
    cout << "          ROUND ROBIN SCHEDULER           \n";
    cout << "==========================================\n";
    int n = getValidatedInt("Enter number of processes: ", 1);
    int tq = getValidatedInt("Enter Time Quantum: ", 1);
    RoundRobinSimulator scheduler(n, tq);
    cout << "\n--- ENTER PROCESS DATA ---\n";
    for(int i = 0; i < n; i++)
    {
        int id, at, bt;

        cout << "\nProcess #" << (i + 1) << endl;
        while (true)
        {
            id = getValidatedInt("Process ID (>0 unique): ", 1);

            if (pidExists(scheduler.processList, i, id))
                cout << "PID already exists. Enter unique ID.\n";
            else
                break;
        }
        at = getValidatedInt("Arrival Time (>=0): ", 0);
        bt = getValidatedInt("Burst Time (>0): ", 1);

        scheduler.addProcess(i, id, at, bt);
    }

    scheduler.run();
    return 0;
}
