#!/usr/bin/env python3
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas as pd
from scipy import stats
import sys
from matplotlib import cm

barwidth = 0.1


# make hatches less annoyingly thick
mpl.rcParams['hatch.linewidth'] = 0.5

mpl.rcParams["errorbar.capsize"] = 2

BIGGER_SIZE = 24
plt.rc('font', size=BIGGER_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=BIGGER_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=BIGGER_SIZE-1)     # fontsize of the x and y labels
plt.rc('xtick', labelsize=BIGGER_SIZE-2)    # fontsize of the tick labels
plt.rc('ytick', labelsize=BIGGER_SIZE-2)    # fontsize of the tick labels
plt.rc('legend', fontsize=BIGGER_SIZE)

data_dir  = sys.argv[1] # "../data/fib-20"
fname     = sys.argv[2]
modes     = ["fib16", "fib32", "fib64"]
files  = [x + ".csv" for x in modes]


groups   = 2
modes    = 3
bar_pos  = [barwidth * 2*x for x in np.arange(3)]
bar_pos1 = [x + barwidth for x in bar_pos]

y = [pd.read_csv(data_dir + "/" + x, comment='#', names=['trial', 'latency'])['latency'].values for x in files]

y_filtered = []
y_opt_filtered   = []
for exp,exp_new in [(y, y_filtered)]:
    for mode in exp:
        print(mode)
        d_25  = np.quantile(mode, 0.25)
        iqr   = stats.iqr(mode)
        d_75  = np.quantile(mode, 0.75)
        lower = d_25 - 1.5*iqr
        upper = d_75 + 1.5*iqr
        cnew = []
        for x in mode:
            if x > lower and x < upper:
                cnew.append(x)
        exp_new.append(np.array(cnew))


y_means = [np.mean(x) for x in y_filtered]

y_std = [np.std(x) for x in y_filtered]

fig, ax = plt.subplots(1, figsize=(9,5))

hatches = ['/', 'o']

color = cm.viridis(np.linspace(0.3, 0.6, modes))

plt.bar(bar_pos, y_means, zorder=3, yerr=y_std, color=color, width=barwidth, label='unopt', linewidth=1.5, edgecolor='black', alpha=0.8, capsize=10)

ax.set_xticks(bar_pos)

#ax.legend(loc='upper center', fontsize=BIGGER_SIZE-2, ncol=2)
ax.set_ylabel(r'Latency (cycles)')
# ax.set_ylim(70, 85)
ax.grid(zorder=0, alpha=0.5, axis='y', which='major')
ax.set_xticklabels(['real (16)', 'prot. (32)', 'long (64)'])
plt.tight_layout()
plt.savefig(fname)
