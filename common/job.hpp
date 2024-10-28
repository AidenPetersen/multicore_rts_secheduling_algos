#pragma once

enum JobType { periodic, aperiodic }

class Job {
public:
	JobType type;
	double wcet;
	double deadline;
	double period;
}
