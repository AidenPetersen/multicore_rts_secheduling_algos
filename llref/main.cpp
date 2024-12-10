#include <iostream>
#include <vector>
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
    // double w1 = (j1.entry_time + j1.deadline) - time;
    // double b1 = w1 * j1.remaining_time / j1.deadline;

    // double w2 = (j2.entry_time + j2.deadline) - time;
    // double b2 = w2 * j2.remaining_time / j2.deadline;
    // return b1 > b2;
    double v1 = -j1.remaining_time / (j1.entry_time + j1.deadline - time);

    double v2 = -j2.remaining_time / (j2.entry_time + j2.deadline - time);
    return v1 <= v2;
}

class LLREF
{
private:
    std::vector<Job> not_selected_jobs;
    std::vector<Job> selected_jobs;
    std::vector<Job> inactive_jobs;
    int processors;
    double time_now;

public:
    LLREF(std::vector<Job> jobs, int np)
    {
        processors = np;
        time_now = 0;
        inactive_jobs = jobs;
    }

    void run()
    {

        schedule(true);
        while (!(inactive_jobs.empty() && selected_jobs.empty() && not_selected_jobs.empty()))
        {
            schedule(false);
        }
    }

    double next_event(event_type *event)
    {

        double next_floor;
        double next_ceiling;
        if (!selected_jobs.empty())
        {
            // Get selected val that will end first
            double min = __DBL_MAX__;
            for (int i = 0; i < (int)selected_jobs.size(); i++)
            {
                double rem = selected_jobs[i].remaining_time + time_now;
                min = rem < min ? rem : min;
            }
            printf("[%0.2f] Next floor: %f\n", time_now, min);
            next_floor = min;
        }
        else
        {
            next_floor = __DBL_MAX__;
        }
        if (!not_selected_jobs.empty())
        {
            // Get val that will hit slope first
            double min = __DBL_MAX__;
            int min_id = -1;
            for (int i = 0; i < (int)not_selected_jobs.size(); i++)
            {
                double rem = not_selected_jobs[i].entry_time + not_selected_jobs[i].deadline - not_selected_jobs[i].remaining_time;
                if (rem < min)
                {
                    min = rem;
                    min_id = not_selected_jobs[i].id;
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
        for (int i = 0; i < (int)inactive_jobs.size(); i++)
        {
            double entry_time = inactive_jobs[i].entry_time;
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
        for (int i = 0; i < (int)inactive_jobs.size(); i++)
        {
            if (inactive_jobs[i].entry_time <= next_time + EPSILON)
            {
                printf("[%0.2lf] Job ID: %d has been activated\n", next_time, inactive_jobs[i].id);
                not_selected_jobs.push_back(inactive_jobs[i]);
                inactive_jobs.erase(inactive_jobs.begin() + i);
                i--;
            }
        }
        // update remaining time on active jobs
        // remove them if complete
        auto aj = selected_jobs.begin();
        while (aj != selected_jobs.end())
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
                aj = selected_jobs.erase(aj);
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

        std::vector<Job> sorted_active;
        sorted_active.insert(sorted_active.end(), selected_jobs.begin(), selected_jobs.end());
        sorted_active.insert(sorted_active.end(), not_selected_jobs.begin(), not_selected_jobs.end());

        sort(sorted_active.begin(), sorted_active.end(), lret);

        printf("[%0.2lf] SORTED: ", time_now);
        for (auto j : sorted_active)
        {
            printf("%d ", j.id);
        }
        printf("\n");

        auto sab = sorted_active.begin() + processors;
        if (sab > sorted_active.end())
        {
            sab = sorted_active.end();
        }

        if (sorted_active.size() > 0)
        {
            selected_jobs = std::vector<Job>(sorted_active.begin(), sab);
            not_selected_jobs = std::vector<Job>(sab, sorted_active.end());
        }

        printf("[%0.2lf] Job IDs: ", time_now);
        for (auto j : selected_jobs)
        {
            printf("%d ", j.id);
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