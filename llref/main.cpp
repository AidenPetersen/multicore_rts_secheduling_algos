#include <iostream>
#include <vector>
#include "job.hpp"

int main()
{
    std::cout << "hi" << std::endl;
}

// FORMAT
// NUM_PERIODIC NUM_APERIODIC RUNTIME
// PERIODIC_TASKS : C P D
// APERIODIC_TASKS: C D ENTRY_TIME
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
        std::cin >> c >> p >> d;
        for (double j = 0; j < runtime; j += p)
        {
            jobs.push_back(Job(c, d, j));
        }
    }

    for (int i = 0; i < na; i++)
    {
        double c, d, e;
        std::cin >> c >> d >> e;
        jobs.push_back(Job(c, d, e));
    }

    return jobs;
}

double static next_event(std::vector<Job>& selected, std::vector<Job>& not_selected)
{
    double next_floor;
    double next_ceiling;
    if (!selected.empty())
    {
        // Get selected val that will end first
        next_floor = selected.back().remaining_time;
    }
    else
    {
        next_floor = __DBL_MAX__;
    }

    if (!not_selected.empty())
    {
        // Get val that will hit slope first
        next_ceiling = not_selected[0].remaining_time;
    }
    else
    {
        next_ceiling = __DBL_MAX__;
    }
    return next_floor > next_ceiling ? next_ceiling : next_floor;
}

void schedule()
{
    // start time is past current time
    std::vector<Job> active;
    // is running
    std::vector<Job> selected;
    // is not running, but is active
    std::vector<Job> not_selected;
}
