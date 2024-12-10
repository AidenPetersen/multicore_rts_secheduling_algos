#pragma once

class Job {
public:
	int id;
	double wcet;
	double deadline;
	double remaining_time;
	double entry_time;

	Job(int i, double w, double dline, double e) : id(i), wcet(w), deadline(dline), entry_time(e) {
		remaining_time = wcet;
	 }

};
