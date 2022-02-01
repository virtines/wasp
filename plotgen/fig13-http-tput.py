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

BIGGER_SIZE = 19
plt.rc('font', size=BIGGER_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=BIGGER_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=BIGGER_SIZE)     # fontsize of the x and y labels
plt.rc('xtick', labelsize=BIGGER_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=BIGGER_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=BIGGER_SIZE)

fname     = sys.argv[2]

data_dir  = sys.argv[1]
files = ['http_baseline_thru.csv', 'http_virtine_thru.csv', 'http_virtine_snapshot_thru.csv']

bar_pos  = np.arange(3)

y = [(1000/pd.read_csv(data_dir + "/" + x, comment='#', names=['trial', 'microseconds'])['microseconds'].values)*1000000 for x in files]
y_means = [stats.hmean(x) for x in y]
#y_std = [np.std(x) for x in y]
print(y_means)

fig, ax = plt.subplots(1, figsize=(5,5))

hatches = ['/', 'o', '-']

color = cm.viridis(np.linspace(0.3, 0.9, 3))

plt.bar(bar_pos[0], y_means[0], align='edge', hatch=hatches[0]*3, color=color[0], zorder=3, width=barwidth, label='native', linewidth=0.25, edgecolor='black')
plt.bar(bar_pos[1], y_means[1], align='edge', hatch=hatches[1]*3, color=color[1], zorder=3, width=barwidth, label='virtine', linewidth=0.25, edgecolor='black')
plt.bar(bar_pos[2], y_means[2], align='edge', hatch=hatches[2]*3, color=color[2], zorder=3, width=barwidth, label='virtine SP', linewidth=0.25, edgecolor='black')

ax.set_xticks([r + barwidth/2 for r in range(0, 3)])

#ax.legend(loc='upper left', fontsize=BIGGER_SIZE-2, ncol=2)
ax.set_ylabel('Throughput (requests/sec)')
#ax.set_ylim(65000, None)
# ax.set_yticks([0, 2000, 4000, 6000, 8000])
# ax.set_yticklabels(['0', '2K', '4K', '6K', '8K'])
ax.set_xticklabels(['native', 'virtine', 'snapshot'])
ax.grid(alpha=0.5, zorder=0, axis='y', which='major')
plt.tight_layout()
plt.savefig(fname)
