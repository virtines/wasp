#!/usr/bin/env python3
import matplotlib as mpl
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import sys
from matplotlib import cm

# make hatches less annoyingly thick
mpl.rcParams['hatch.linewidth']  = 0.5
barwidth = 0.25

data_dir = sys.argv[1]

platforms = ['baseline', 'virtine', 'virtine+snapshot']
fibargs   = ['0', '5', '10', '15', '20', '25', '30']

a = []

# keyed by n
speedups = {}

for n in fibargs:
    data = {}

    data['n'] = [n for i in range(0, 1000)]
    for plat in platforms:
        df = pd.read_csv(f"{data_dir}/{plat}_{n}.csv", sep='\s*,\s*', engine='python')
        data[plat] = df['latency'].tolist()
        # a.append(df[1])
    df = pd.DataFrame(data)


    # print(n, np.mean(df['virtine+snapshot'] / np.mean(df['baseline'])))

    # uncomment to get speedup instead of latency
    # df['virtine_snapshot'] = df['baseline'] / df['virtine_snapshot']
    # df['baseline'] =         df['baseline'] / df['baseline']
    a.append(df)


# exit()

pos_base = np.arange(len(fibargs))
bar_pos = {}

colors = cm.viridis(np.linspace(0.5, 0.9, len(platforms)))

# print(colors)

means         = {}
stddevs       = {}
hatches = ['/', '-', '\\' , '*', 'o', '#', '-', 'O']
# f, ax = plt.subplots(1, figsize=(12,4))
f, ax = plt.subplots(1, figsize=(5,4))

for i, key in enumerate(platforms):
    means[key]   = [np.mean(a[fibargs.index(f)][key].values) for f in fibargs]
    stddevs[key] = [np.std(a[ fibargs.index(f)][key].values) for f in fibargs]
    bar_pos[key] = [x + i * barwidth for x in pos_base]



for i, key in enumerate(platforms):
    bar = ax.bar(bar_pos[key], means[key], label=key, hatch=3*hatches[i], width=barwidth, color=colors[i], edgecolor='black', linewidth=0.25)
    if key == 'virtine':
        for rect in bar:
            height = rect.get_height()
            # plt.text(rect.get_x() + rect.get_width()/2.0, height, '%d' % int(height), ha='center', va='bottom')
        # ax.text(
    l2, caps, trash = ax.errorbar(bar_pos[key], means[key], yerr=stddevs[key], lolims=True, capsize=3, ls='None', color='black')
    for cap in caps:
        cap.set_marker("_")

ax.set_xticks([r + (barwidth*len(platforms)/2) - barwidth/2 for r in range(len(fibargs))])
ax.set_xticklabels(fibargs)
ax.tick_params(axis="both", labelsize=12)
ax.set_xlabel("Computational intensity (arg. to fib)", fontsize=12)
ax.set_ylabel("Mean call latency ($\mu$s)", fontsize=12)
#ax.legend(bbox_to_anchor=(1, 1.15), fontsize=11, ncol=len(platforms))
ax.legend( bbox_to_anchor=(0, 1), loc="upper left", fontsize=11, ncol=1)
ax.grid(axis='y', zorder=-1, alpha=0.25)
ax.set_axisbelow(True)
plt.yscale('log')
plt.tight_layout()
plt.savefig(sys.argv[2])
