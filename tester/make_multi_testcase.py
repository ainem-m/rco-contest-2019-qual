import subprocess
from concurrent.futures import ThreadPoolExecutor
from concurrent.futures import ProcessPoolExecutor


def run(i: int):
    with open(f"../in/{i:04d}.txt", mode="w") as f:
        subprocess.Popen(
            f"java Generator -{seeds[i]}", stdout=f, shell=True)
    # stdout=の指定がないとターミナルにリダイレクトとは別に結果が出力される（なぜ？）


seeds = []
with open("../seeds.txt") as f:
    seeds = list(f.readlines())

with ThreadPoolExecutor() as executor:
    # for i in range(1000):
    #    executor.submit(run(i))
    future = executor.map(run, range(1000))


print("fin")
