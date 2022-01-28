#!/usr/bin/env python3
import csv
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
import sys

BIGGER_SIZE = 24
plt.rc('font', size=BIGGER_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=BIGGER_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=BIGGER_SIZE-2)     # fontsize of the x and y labels
plt.rc('xtick', labelsize=BIGGER_SIZE-3)    # fontsize of the tick labels
plt.rc('ytick', labelsize=BIGGER_SIZE-2)    # fontsize of the tick labels
plt.rc('legend', fontsize=BIGGER_SIZE)


def read_into_df(csv):
    df = pd.read_csv(csv, names=['trial','latency'], comment='#')
    return df

def do_raw_plot(data_dir, abs_list, trials):
    ind   = np.arange(trials) # the x locations for the groups
    width = 1       # the width of the bars: can also be len(x) sequence

    plt_list = []

    for i in abs_list:
        x = read_into_df(f"{data_dir}/{i}.csv")
        plt_list.append(plt.plot(ind, x, width))

    plt.ylabel('Latency (cycles)')
    plt.xlabel('Trial')
    plt.legend(abs_list)

    plt.show()

def list_as_df(data_dir, abs_list, xlabs, trials):
    dfs = []
    for i, tp in enumerate(abs_list):
        df1 = read_into_df(f"{data_dir}/{tp}.csv")
        df1.insert(2, 'type', xlabs[i])
        dfs.append(df1)

    df = pd.DataFrame(np.concatenate([x.values for x in dfs]), columns=dfs[0].columns)
    df["latency"] = df["latency"].astype('float64') # needs to be float for seaborn (?)
    return df

def save_or_show(save=False, filename="plot.pdf"):
    if save:
        plt.savefig(filename)
    else:
        plt.show()


def do_box_plot(data_dir, abs_list, xlabs, trials, save=False, fname="plots.pdf"):
    fig, ax = plt.subplots(figsize=(10,5))
    df = list_as_df(data_dir, abs_list, xlabs, trials)
    sns.boxplot(y="type", x="latency", data=df, palette='Greens', fliersize=0, width=0.5,linewidth=1)
    ax.set_xlabel("Latency (cycles)")
    ax.set_ylabel("")
    plt.xscale('log')
    ax.set_yticklabels(xlabs)
    plt.tight_layout()
    save_or_show(save, fname)
    

data = [
    ('linux_process', 'Process'),
    ('wasp_create', 'Wasp'),
    ('linux_thread', 'pthread'),
    ('wasp_create_cache', 'Wasp+C'),
    ('wasp_create_cache_async', 'Wasp+CA'),
    ('wasp_vmrun', 'vmrun')
]

do_box_plot(sys.argv[1], list(map(lambda d: d[0], data)), list(map(lambda d: d[1], data)), 1000, save=True, fname=sys.argv[2])

