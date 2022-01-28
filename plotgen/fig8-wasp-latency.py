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

def do_jitter_strip_plot(data_dir, abs_list, xlabs, trials, save, fname="plots.pdf"):
    df = list_as_df(data_dir, abs_list, xlabs, trials)
    ax = sns.catplot(x='type', y='latency', data=df, jitter=True, dodge=True, alpha=0.25, edgecolor='gray', linewidth=.2,palette='Blues')
    plt.tight_layout()
    save_or_show(save, fname)

def do_bar_plot(data_dir, abs_list, xlabs, trials, save=False, fname="plots.pdf"):
    fig, ax = plt.subplots()
    df = list_as_df(data_dir, abs_list, xlabs, trials)
    sns.barplot(x="type", y="latency", data=df, capsize=.2, palette='Blues', errwidth=.5)
    ax.tick_params(axis='x', rotation=30)
    plt.tight_layout()
    save_or_show(save, fname)

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
    
def do_violin_plot(data_dir, abs_list, xlabs, trials, save=False, fname="plots.pdf"):
    fig, ax = plt.subplots(1, figsize=(8,7))
    df = list_as_df(data_dir, abs_list, xlabs, trials)
    #df = df.loc[df.latency < 100000]
    sns.violinplot(x="type", y="latency", data=df,  widths=2, linewidth=1, alpha=0.6, palette='magma', cut=0)
    ax.set_xticklabels(xlabs)
    ax.set_ylabel("Latency (cycles)")
    ax.set_xlabel(None)
    plt.yscale('log')
    plt.tight_layout()
    save_or_show(save, fname)

def do_box_swarm_plot(data_dir, abs_list, xlabs, trials, save=False, fname="plots.pdf"):
    fig, ax = plt.subplots()
    df = list_as_df(data_dir, abs_list, xlabs, trials)
    sns.boxplot(x="type", y="latency", data=df, palette='Blues')
    sns.swarmplot(x="type", y="latency", data=df, edgecolor='gray')
    ax.tick_params(axis='x', rotation=30)
    ax.tick_params(axis='x', rotation=30)
    ax.set_xticklabels(xlabs)
    plt.tight_layout()
    save_or_show(save, fname)

def do_box_strip_plot(data_dir, abs_list, xlabs, trials, save=False, fname="plots.pdf"):
    fig, ax = plt.subplots()
    df = list_as_df(data_dir, abs_list, xlabs, trials) 
    sns.boxplot(x="type", y="latency", data=df, palette='Blues')
    sns.stripplot(x="type", y="latency", data=df, edgecolor='black', palette='Blues', alpha=0.24, jitter=True, dodge=True)
    ax.tick_params(axis='x', rotation=30)
    ax.set_xticklabels(xlabs)
    plt.tight_layout()
    save_or_show(save, fname)



def do_strip_plot(data_dir, abs_list, xlabs, trials, save=False, fname="plots.pdf"):
    sns.set_theme(style="whitegrid")
    fig, ax = plt.subplots(1, figsize=(6, 4))
    df = list_as_df(data_dir, abs_list, xlabs, trials) 


    sns.stripplot(x="type", y="latency", data=df, edgecolor='black', palette='viridis', alpha=0.5, jitter=0.4, dodge=True)
    # ax.tick_params(axis='x', rotation=30)
    ax.set_xticklabels(xlabs)
    ax.set_ylabel("Latency (cycles)")
    ax.set_xlabel(None)
    plt.yscale('log')
    plt.tight_layout()
    save_or_show(save, fname)

fname = sys.argv[1]
do_box_plot('../data/abstraction_latencies', ['linux_process', 'wasp_latency_nocache', 'linux_thread', 'wasp_latency_cache', 'wasp_latency_cache_async_cleanup', 'wasp_vmrun'],
         ['Process', 'Wasp', 'pthread', 'Wasp+C', 'Wasp+CA', 'vmrun'],
         1000, save=True, fname=sys.argv[1])

