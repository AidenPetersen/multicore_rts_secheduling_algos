import random
cores = random.randint(1, 32)
coresum = 0
utilization = 0
tasks = []
taskcounter = 0
while True:
    deadline = period = random.randint(1, 1000)
    wcet = random.randint(1, deadline)
    entrytime = 0
    if coresum + wcet / deadline >= cores:
        break
    coresum += wcet / deadline
    taskcounter += 1
    tasks.append([wcet, period, deadline, taskcounter])
print(f"{cores} {taskcounter} {0} {100}")
for t in tasks:
    print(f"{t[0]} {t[1]} {t[2]} {t[3]}")
