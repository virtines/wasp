#!/usr/bin/env python3
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas as pd
from scipy import stats
import sys
from matplotlib import cm

barwidth = 0.5

# make hatches less annoyingly thick
mpl.rcParams['hatch.linewidth'] = 0.5

mpl.rcParams["errorbar.capsize"] = 2

BIGGER_SIZE = 17
plt.rc('font', size=BIGGER_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=BIGGER_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=BIGGER_SIZE)     # fontsize of the x and y labels
plt.rc('xtick', labelsize=BIGGER_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=BIGGER_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=BIGGER_SIZE)

NBARS = 4

data_dir  = sys.argv[1]
files = ['baseline.csv', 'virtine.csv', 'virtine_noteardown.csv', 'virtine_snapshot.csv', 'virtine_snapshot_noteardown.csv']
fname = sys.argv[2]

bar_pos  = np.arange(NBARS)

y = [pd.read_csv(data_dir + "/" + x, comment='#', names=['trial', 'latency'])['latency'].values for x in files]
y_means = [np.mean(x) for x in y]

print(y_means)

fig, ax = plt.subplots(1, figsize=(6, 3.5))

hatches = ['/', 'o', '\\', '-']

color = cm.viridis(np.linspace(0.4, 0.9, NBARS))

labels=['Virtine', 'Virtine NT', 'Virtine + Snapshot', 'Virtine + Snapshot+NT']

for i in range(NBARS):
    plt.bar(bar_pos[i], 
            y_means[i+1]/y_means[0],
            align='edge',
            hatch=hatches[i]*2,
            color=color[i],
            width=barwidth,
            zorder=3,
            label=labels[i],
            linewidth=0.25,
            edgecolor='black')
    plt.text(bar_pos[i], y_means[i+1]/y_means[0] + 0.05, str(int(y_means[i+1])) + r"$\mu$s", fontsize=BIGGER_SIZE-4)

ax.set_xticks([r + barwidth/2 for r in range(NBARS)])

ax.legend(loc='lower left', bbox_to_anchor=(0, 1.02), fontsize=BIGGER_SIZE-4, ncol=2)
ax.set_ylabel('Slowdown')
# ax.set_ylim(0,1.5)
ax.grid(alpha=0.5, zorder=0, which='major', axis='y')
ax.set_xticklabels("", rotation=90)
plt.tight_layout()
plt.savefig(fname)
