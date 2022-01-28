#!/usr/bin/env python3
import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas as pd
import sys

# mpl.use('MACOSX')

mpl.rcParams["errorbar.capsize"] = 2

BIGGER_SIZE = 23
plt.rc('font', size=BIGGER_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=BIGGER_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=BIGGER_SIZE)     # fontsize of the x and y labels
plt.rc('xtick', labelsize=BIGGER_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=BIGGER_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=BIGGER_SIZE)


data_dir  = sys.argv[1]
fname = sys.argv[2]
input_file = 'image_size.csv'

y = pd.read_csv(data_dir + "/" + input_file, comment='#', names=['trial', 'bytes', 'microseconds'])

means = []
stddevs = []
for b in y['bytes'].unique():
    means.append(np.mean(y['microseconds'][y['bytes'] == b].values))
    stddevs.append(np.std(y['microseconds'][y['bytes'] == b].values))


fig, ax = plt.subplots(1, figsize=(12,6))

x = np.linspace(0, 1, len(y['bytes'].unique()))
xlabs = ['32B', '64B', '128B', '256B', '512B', '1K', '2K', '4K', '8K', '16K', '32K', '64K', '128K', '265K', '512K', '1M', '2M', '4M', '8M', '16M']

plt.xticks(x, xlabs)
ax.errorbar(x, means, yerr=stddevs, lw=4,marker='o', markerfacecolor='white',ms=14, markeredgewidth=4,ecolor='black',capsize=6, zorder=3)
ax.tick_params(axis='x', rotation=90)

ax.grid(visible=True, which='both', axis='y', alpha=0.5, zorder=0)

ax.set_ylabel('Latency (ms)')
# ax.set_yticks([0, 500, 1000, 1500, 2000, 2500])
ax.set_yticklabels(['0.0', '0.5', '1.0', '1.5', '2.0', '2.5'])
ax.set_xlabel('Virtine image Size')
plt.tight_layout()
plt.savefig(fname)
