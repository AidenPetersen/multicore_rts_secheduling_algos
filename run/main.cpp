#include<iostream>
#include <vector>
#include <map>
#include <bits/stdc++.h>
#include "job.hpp"

class ProperSubsystem;
class Server;


std::priority_queue<double, std::vector<double>, std::greater<>> trigger_times;
std::vector<Server*> sel_jobs(Server* s, std::vector<Server*>& v, int exec, double curr_time);
std::vector<Job> cpus;
int id_ctr = -1;

bool eq(double a, double b){
    if(fabs(a-b) < 0.000001){
        return true;
    } else {
        return false;
    }
}


class Server{
public:
    std::vector<Server*> children;
    Server* parent;
    int dual = 0;
    int is_task = 0;
    int task_id = -1;
    std::multiset<double> deadlines;
    double budget;
    double next_deadline;
    double util;
    double progress=0;
    int last_iter = -1;

    double c,p,d;

    Server(){
        util=0;
        budget=0;
        next_deadline =0;
        task_id = --id_ctr;
    };

    Server(double ci, double pi, double di, int id){
        c = ci;
        p = pi;
        d = di;
        task_id = id;
        is_task=1;
        util = c/p;
        renew_budget(0);
    }

    void make_progress(double curr_time){
        progress+=1;

        if(progress==c){
            int iter = (int)(curr_time/p);
            printf("Finished task id %d iter %d at %0.2f\n", task_id, iter, curr_time);
            if(iter - last_iter != 1){
                printf("Task %d seems to have skipped iteration\n", task_id);
            }
            last_iter = iter;
            progress=0;
        }

    }


    /*void add_deadline(Job task){
        deadlines.push()
    }*/

    void renew_budget(double curr_time){
        int iter = (int)(curr_time/p);
        budget = util*((p+1)*iter - curr_time);
    }

    bool is_active(double curr_time){
        if(is_task){
            int iter = (int)(curr_time/p);
            return iter > last_iter;
        } else{
            bool a = false;
            for(auto x : children){
                a = a || x->is_active(curr_time);
            }
            return a;
        }

    }

    double get_next_deadline(){
        if(is_task){
            return (last_iter+2)*p;
        } else {
            double mind = 999999999;
            for(auto x : children){
                mind = fmin(mind, x->get_next_deadline());
            }
            return mind;
        }
    }

    void add_child(Server* x){
        children.push_back(x);
        util += x->util;
        x->parent= this;
    }

    Job make_job(double curr_time){
        int iter = (int)(curr_time/p);
        Job j(task_id, c, d + (p*iter), p*iter);
        return j;
    }




};

void print_vec(std::vector<Server*> x, double curr_time){
    printf("-------Jobs at %0.1f--------------\n", curr_time);
    for(auto j : x){
        printf("ID: %d\n", j->task_id);
    }
}



class ProperSubsystem{
public:
    Server* rootserver;
    std::vector<int> cpus;
    double last_update = 0;
    std::vector<Server*> servers;


    ProperSubsystem(Server* root){
        rootserver=root;
        printf("Made a subsystem\n");
    };

    void update(double current_time){
        double deltat = current_time - last_update;
        for(auto x : servers){
            x->budget -= deltat;
        }
        last_update = current_time;

    }

    std::vector<Server*> schedule(double curr_time){
        std::vector<Server*> runjobs;

        servers = std::vector<Server*>();
        std::vector<Server*> jobs;
        jobs = sel_jobs(rootserver, servers, 1, curr_time);
        return jobs;

    }



};

std::vector<Server*> get_tasks(Server* s){
    std::vector<Server*> j;
    if(s->is_task){
        j.push_back(s);
    } else {
        for(auto x: s->children){
            std::vector<Server*> c = get_tasks(x);
            j.insert(j.end(), c.begin(), c.end());
        }

    }
    return j;

}

Server* dual_server(Server* c){
    Server* d = new Server();
    d->children.push_back(c);
    d->dual= 1;
    d->util = 1- c->util;
    c->parent = d;
    return d;

}


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

        jobs.push_back(Job(id, c, d, 0));

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

class RUNScheduler{
public:
    std::vector<Server*> tasks;
    std::vector<Server*> tasks_flat;

    std::vector<ProperSubsystem*> subsystems;
    std::map<int, ProperSubsystem*> job_to_subsystem;
    int nproc;


    RUNScheduler(int n, std::vector<Job> t){
        nproc = n;
        tasks = make_task_servers(t);
        reduce(tasks);
        for(auto x : subsystems){
            auto flat = get_tasks(x->rootserver);
            tasks_flat.insert(tasks_flat.begin(), flat.begin(), flat.end());
        }
    }

    std::vector<Server*> make_task_servers(std::vector<Job> jobs){
        std::vector<Server*> s;
        for(auto x : jobs){
            s.push_back(new Server(x.wcet, x.deadline, x.deadline, x.id));
        }
        return s;
    }


    void run(double max_time){
        double curr_t = 0;
        while(curr_t < max_time){
            std::vector<Server*> jobs = schedule(curr_t);
            print_vec(jobs, curr_t);
            for(auto x : jobs){
                x->make_progress(curr_t);
            }
            curr_t++;

        }

    }

    std::vector<Server*> schedule(double curr_time){
        std::vector<Server*> j;
        for(auto x: subsystems){
            std::vector<Server*> sj = x->schedule(curr_time);
            j.insert(j.end(), sj.begin(), sj.end());
        }
        return j;
    }


    void make_idle_tasks(std::vector<Server*>& tsk){
        double tutil=0;
        for(auto s : tsk){
            tutil += s->util;
        }
        double idletime = (1.0*nproc) - tutil;
        for(auto s : tsk){
            if(s->util < 1 && idletime>0){
                Server* idletask = new Server(999999999, 999999999, 999999999, -1);
                idletask->util = fmin(1- s->util, idletime);
                s->add_child(idletask);
                idletime -= idletask->util;
            }
        }
        while(idletime>0){
            Server* idletask = new Server(999999999, 999999999, 999999999, -1);
            idletask->util=1;
            idletime -= idletask->util;
            tsk.push_back(idletask);

        }


    }

    void make_subsystems(std::vector<Server*> servers){
        std::vector<Server*> s;
        for(auto x: servers){
            if(eq(x->util, 1)){
                make_subsystem(x);
            } else {
                s.push_back(x);
            }
        }
        tasks=s;

    }

    void make_subsystem(Server* s){

        std::vector<Server*> tasks = get_tasks(s);
        ProperSubsystem* system = new ProperSubsystem(s);
        for(Server* x: tasks){
            job_to_subsystem[x->task_id] = system;
        }
        subsystems.push_back(system);

    }


   void reduce(std::vector<Server*> servers) {

        tasks = pack(tasks);
        make_idle_tasks(tasks);
        make_subsystems(tasks);

        while(tasks.size()>0){
            tasks = pack(dual(tasks));
            make_subsystems(tasks);
        }


    }

    std::vector<Server*> pack(std::vector<Server*> servers){
        std::vector<Server*> packed;
        for(auto& x : servers){
            int added = 0;
            for(auto& y: packed){
                if(y->util + x->util <= 1) {
                    y->add_child(x);
                    added=1;
                    break;
                }
            }
            if(!added){
                Server* y = new Server();
                y->add_child(x);
                packed.push_back(y);
            }

        }
        return packed;
    }

    std::vector<Server*> dual(std::vector<Server*> servers) {
        std::vector<Server*> duals;
        for(auto s : servers){
            duals.push_back(dual_server(s));
        }
        return duals;

    }




};




std::vector<Server*> sel_jobs(Server* s, std::vector<Server*>& v, int exec, double curr_time){
    std::vector<Server*> jobs;
    if(exec){
        v.push_back(s);
    }

    if(s->is_task){
        if(exec && s->is_active(curr_time)){
            jobs.push_back(s);
        }
    } else {
        if(s->dual){
            std::vector<Server*> newjobs = sel_jobs(s->children[0], v, !exec, curr_time);
            jobs.insert(jobs.end(), newjobs.begin(), newjobs.end());
        } else {
            Server* mins;
            double min_d=9999999999;
            for(Server* x : s->children){
                if(x->is_active(curr_time)){
                    double d = x->get_next_deadline();
                    if(d<min_d){
                        min_d = d;
                        mins = x;
                    }
                }
            }

            for(Server*x : s->children){
                std::vector<Server*> newjobs = sel_jobs(x, v, (exec && (x==mins)), curr_time);
                jobs.insert(jobs.end(), newjobs.begin(), newjobs.end());
            }


        }

    }
    //print_vec(jobs, curr_time);
    return jobs;
}






int main() {
    int num_cpus;
    std::cin >> num_cpus;
    std::vector<Job> jobs = parse_jobs();
    RUNScheduler run(num_cpus, jobs);
    run.run(1000);




}