#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include "job.hpp"
#define EPSILON 0.00000001

enum event_type
{
    FLOOR,
    CEILING,
    NEW_TASK
};

// Calculate local remaining execution time
bool lret_comp(Job j1, Job j2, double time)
{
    double v1 = -j1.remaining_time / (j1.entry_time + j1.deadline - time);

    double v2 = -j2.remaining_time / (j2.entry_time + j2.deadline - time);
    return v1 <= v2;
}

class LLREF
{
private:
    std::vector<Job>* sorted_active_jobs;
    std::vector<Job>* inactive_jobs;
    int processors;
    double time_now;

public:
    LLREF(std::vector<Job> jobs, int np)
    {

        sorted_active_jobs = new std::vector<Job>();
        inactive_jobs = new std::vector<Job>(jobs);
        processors = np;
        time_now = 0;
    }
    ~LLREF(){
        delete(sorted_active_jobs);
        delete(inactive_jobs);
    }

    void run()
    {

        schedule(true);
        while (!(inactive_jobs->empty() && sorted_active_jobs->empty()))
        {
            schedule(false);
        }
    }

    double next_event(event_type *event)
    {

        double next_floor;
        double next_ceiling;
        if (!sorted_active_jobs->empty())
        {
            // Get selected val that will end first
            double min = __DBL_MAX__;
            for (int i = 0; i < (int)sorted_active_jobs->size() && i < (int)processors; i++)
            {
                double rem = sorted_active_jobs->at(i).remaining_time + time_now;
                min = rem < min ? rem : min;
            }
            printf("[%0.2f] Next floor: %f\n", time_now, min);
            next_floor = min;
        }
        else
        {
            next_floor = __DBL_MAX__;
        }
        if ((int)sorted_active_jobs->size() > processors)
        {
            // Get val that will hit slope first
            double min = __DBL_MAX__;
            int min_id = -1;
            for (int i = processors; i < (int)sorted_active_jobs->size(); i++)
            {
                double rem = sorted_active_jobs->at(i).entry_time + sorted_active_jobs->at(i).deadline - sorted_active_jobs->at(i).remaining_time;
                if (rem < min)
                {
                    min = rem;
                    min_id = sorted_active_jobs->at(i).id;
                }
            }
            printf("[%0.2f] Next ceiling: %f on ID: %d\n", time_now, min, min_id);
            next_ceiling = min;
        }
        else
        {
            next_ceiling = __DBL_MAX__;
        }
        double next_collision;
        if (next_floor > next_ceiling && next_ceiling != time_now)
        {
            next_collision = next_ceiling;
            *event = CEILING;
        }
        else
        {
            next_collision = next_floor;
            *event = FLOOR;
        }
        // Find next instruction to be issued
        for (int i = 0; i < (int)inactive_jobs->size(); i++)
        {
            double entry_time = inactive_jobs->at(i).entry_time;
            if (entry_time < next_collision)
            {
                next_collision = entry_time;
                *event = NEW_TASK;
            }
        }
        printf("\n");
        return next_collision;
    }

    void schedule(bool first)
    {
        // Next update time
        // floor as default to stop warnings
        event_type event = FLOOR;
        double next_time = first ? 0.0 : next_event(&event);
        std::string event_s = "";
        switch (event)
        {
        case CEILING:
            event_s = "ceiling";
            break;

        case FLOOR:
            event_s = "floor";
            break;

        case NEW_TASK:
            event_s = "new task";
            break;

        default:
            break;
        }
        printf("[%0.2lf] NEW EVENT %s\n", next_time, event_s.c_str());

        // move new jobs into inactive
        for (int i = 0; i < (int)inactive_jobs->size(); i++)
        {
            if (inactive_jobs->at(i).entry_time <= next_time + EPSILON)
            {
                printf("[%0.2lf] Job ID: %d has been activated\n", next_time, inactive_jobs->at(i).id);
                sorted_active_jobs->push_back(inactive_jobs->at(i));
                inactive_jobs->erase(inactive_jobs->begin() + i);
                i--;
            }
        }
        // update remaining time on active jobs
        // remove them if complete
        auto aj = sorted_active_jobs->begin();
        int aj_counter = processors;
        while (aj < (sorted_active_jobs->begin() + aj_counter) && aj  < sorted_active_jobs->end())
        {
            aj->remaining_time -= (next_time - time_now);
            if (aj->remaining_time <= EPSILON)
            {
                if ((aj->entry_time + aj->deadline) < next_time - EPSILON)
                {
                    printf("[%0.2lf] Job ID: %d has been completed LATE\n", next_time, aj->id);
                }
                else
                {
                    printf("[%0.2lf] Job ID: %d has been completed\n", next_time, aj->id);
                }
                aj = sorted_active_jobs->erase(aj);
                aj_counter--;
            }
            else
            {
                aj++;
            }
        }
        // Update time for sort
        // double old_time = time_now;
        time_now = next_time;
        auto lret = [next_time](Job j1, Job j2) -> bool
        {
            return lret_comp(j1, j2, next_time);
        };

        sort(sorted_active_jobs->begin(), sorted_active_jobs->end(), lret);

        printf("[%0.2lf] SORTED: ", time_now);
        for (auto j : *sorted_active_jobs)
        {
            printf("%d ", j.id);
        }
        printf("\n");

        auto sab = sorted_active_jobs->begin() + processors;
        if (sab > sorted_active_jobs->end())
        {
            sab = sorted_active_jobs->end();
        }

        printf("[%0.2lf] Job IDs: ", time_now);
        for (int i = 0; i < processors && i < (int)sorted_active_jobs->size(); i++)
        {
            printf("%d ", sorted_active_jobs->at(i).id);
        }
        printf("have been scheduled\n");
    }
};

// FORMAT
// NUM_CORES NUM_PERIODIC NUM_APERIODIC RUNTIME
// PERIODIC_TASKS : C P D ID
// APERIODIC_TASKS: C D ENTRY_TIME ID
std::vector<Job> parse_jobs()
{
    int np, na;
    double runtime;
    std::cin >> np >> na >> runtime;

    // vector is not sorted meaningfully
    std::vector<Job> jobs;
    for (int i = 0; i < np; i++)
    {
        double c, p, d;
        int id;
        std::cin >> c >> p >> d >> id;
        for (double j = 0; j < runtime; j += p)
        {
            jobs.push_back(Job(id, c, d, j));
        }
    }

    for (int i = 0; i < na; i++)
    {
        double c, d, e;
        int id;
        std::cin >> c >> d >> e >> id;
        jobs.push_back(Job(id, c, d, e));
    }

    return jobs;
}

void print_jobs(std::vector<Job> jobs)
{
    printf("ALL JOBS\n");
    for (auto j : jobs)
    {
        printf("Job ID: %d, start: %0.2lf, dline: %0.2lf, wcet: %0.2lf\n", j.id, j.entry_time, j.deadline, j.wcet);
    }
    printf("\n");
}

int main()
{
    int cores;
    std::cin >> cores;
    std::vector<Job> jobs = parse_jobs();
    print_jobs(jobs);
    LLREF scheduler(jobs, cores);
    scheduler.run();
}