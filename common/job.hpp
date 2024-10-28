#pragma once

class Job {
public:
	double wcet;
	double deadline;
	double remaining_time;
	double entry_time;

	Job(double w, double dline, double e) : wcet(w), deadline(dline), entry_time(e) {
		remaining_time = deadline;
	 }

	
};
