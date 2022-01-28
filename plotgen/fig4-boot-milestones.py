#!/usr/bin/env python3
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib as mpl
from scipy import stats
import csv
import sys
from matplotlib import cm

BIGGER_SIZE = 18
plt.rc('font', size=BIGGER_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=BIGGER_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=BIGGER_SIZE)     # fontsize of the x and y labels
plt.rc('xtick', labelsize=BIGGER_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=BIGGER_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=BIGGER_SIZE)

barwidth = 0.4
mpl.rcParams["errorbar.capsize"] = 4

def remove_tukey(df, key, constant):
    x25 = np.quantile(df[key].values, 0.25)
    x75 = np.quantile(df[key].values, 0.75)
    iqr = stats.iqr(df[key].values)
    lower = x25 - iqr
    upper = x75 + iqr
    return np.array([x for x in df[key].values if x < upper and x > lower])


def read_files(data_dir, f):
    return pd.read_csv(data_dir + "/" + f + ".csv", comment='#', sep='\s*,\s*', header=0)

means    = {'kmain()': 0, 'after recv()': 0, 'after send()': 0}
stddevs  = {'kmain()': 0, 'after recv()': 0, 'after send()': 0}

a = read_files('../data/boottime', 'echo-server')

for i, k in enumerate(means.keys()):
    means[k]   = np.mean(remove_tukey(a, k, 1.5))
    stddevs[k] = np.std(remove_tukey(a, k, 1.5))

names = ['context_main()', 'recv()', 'send()']

labels = ['0', '25K', '50K', '75K', '100K', '125K', '150K', '175K']


# Create figure and plot a stem plot with the date
fig, ax = plt.subplots(figsize=(8.8, 2), constrained_layout=True)

ax.set_xticklabels(labels)
ax.set_xlabel("Cycles to pass boot milestone")

mean_vals = [v for k,v in means.items()]
std_vals  = [v for k,v in stddevs.items()]
print(mean_vals)
print(std_vals)
ax.vlines(mean_vals, 0, [2,2,2], color="tab:red")  # The vertical stems.
ax.plot(mean_vals, [0,0,0], "-o", color="k", markerfacecolor="w")  # Baseline and markers on it.

ax.set_ylim((-2, 4))
# annotate lines
i = 0
std_strs = ['856', '45.7K', '45.5K']
for d, l, r in zip(mean_vals, [2,2,2], names):
    ax.annotate(r + f"\n$\sigma$={std_strs[i]}", xy=(d, l),
                xytext=(-3, np.sign(l)*3), textcoords="offset points",
                horizontalalignment="center",
                verticalalignment="bottom" if l > 0 else "top")
    i += 1

# format xaxis with 4 month intervals
#plt.setp(ax.get_xticklabels(), rotation=30, ha="right")

# remove y axis and spines
ax.yaxis.set_visible(False)
ax.spines["left"].set_visible(False)
ax.spines["top"].set_visible(False)
ax.spines["right"].set_visible(False)

ax.margins(y=0.1)

#plt.tight_layout()

plt.savefig(sys.argv[1])
